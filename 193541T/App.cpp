#include "App.h"
#include "Generic.h"
#include "Global.h"

GLFWwindow* App::win = 0; //Render window

App::App(): camResetBT(0.f), cullBT(0.f), polyModeBT(0.f), ppeTypeBT(0.f), typePPE(0){
    Init();
    scene = new Scene;
}

App::~App(){
    delete scene;
    frontFBO->Del();
    backFBO->Del();
    dDepthMapFBO->Del();
    sDepthMapFBO->Del();
    enFBO->Del();
    intermediateFBO->Del();
    ShaderProg::ClearShaderCache();
    glfwTerminate(); //Clean/Del all GLFW's resources that were allocated
}

bool App::Key(int key){
    return bool(glfwGetKey(win, key));
}

void App::RenderSceneToCreatedFB(const Cam& cam, const Framebuffer* const& FBO, const Framebuffer* const& enFBO, const uint* const& depthTexs, const short& index) const{ //Create framebuffer (combination of render buffers stored in GPU mem) to get an extra target to render to
    glBindFramebuffer(GL_FRAMEBUFFER, FBO->GetRefID()); {
        if(index != 999){
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, FBO->GetTex().GetRefID(), 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        } else{
            glClear(cam.GetProjectionType() > 1 ? GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT : GL_DEPTH_BUFFER_BIT); //State-using func //Clear info from prev frame stored in each render buffer with the appropriate bits set
        }
        scene->RenderToCreatedFB(cam, enFBO ? &(enFBO->GetTex()) : 0, depthTexs);
    } glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void App::RenderSceneToDefaultFB(const Framebuffer* const& readFBO, const Framebuffer* const& writeFBO, const int& typePPE, const glm::vec3&& translate, const glm::vec3&& scale) const{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBO->GetRefID()); glBindFramebuffer(GL_DRAW_FRAMEBUFFER, writeFBO->GetRefID()); {
        glBlitFramebuffer(0, 0, 800, 600, 0, 0, 800, 600, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    } glBindFramebuffer(GL_FRAMEBUFFER, 0); //Stop off-screen rendering by making the default framebuffer active //Unbind the framebuffer so won't accidentally render to the wrong framebuffer
    glClear(GL_DEPTH_BUFFER_BIT);
    scene->RenderToDefaultFB(writeFBO->GetTex(), typePPE, translate, scale); //Some post-processing filters like edge detection will produce jagged edges as screenTex is non-multisampled so do post-processing later or use anti-aliasing alg to correct
}

void App::Init(){
    InitGL(win);
    frontFBO = new Framebuffer(GL_TEXTURE_2D_MULTISAMPLE, 3, 800, 600, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    backFBO = new Framebuffer(GL_TEXTURE_2D_MULTISAMPLE, 3, 800, 600, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    dDepthMapFBO = new Framebuffer(GL_TEXTURE_2D, 2, 1024, 1024, GL_NEAREST, GL_NEAREST, GL_REPEAT);
    sDepthMapFBO = new Framebuffer(GL_TEXTURE_2D, 2, 1024, 1024, GL_NEAREST, GL_NEAREST, GL_REPEAT);
    enFBO = new Framebuffer(GL_TEXTURE_CUBE_MAP, 1, 1700, 1700, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    intermediateFBO = new Framebuffer(GL_TEXTURE_2D, 0, 800, 600, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
}

void App::Update(const Cam& cam){
    float currFrame = (float)glfwGetTime();
    dt = currFrame - lastFrame;
    lastFrame = currFrame;

    scene->Update(cam);

    if(glfwGetKey(win, GLFW_KEY_R) && camResetBT <= currFrame){
        const_cast<Cam&>(cam).Reset();
        camResetBT = currFrame + .5f;
    }

    if(glfwGetKey(win, GLFW_KEY_1) && cullBT <= currFrame){
        glIsEnabled(GL_CULL_FACE) ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);
        cullBT = currFrame + .5f;
    }

    GLint polyMode;
    glGetIntegerv(GL_POLYGON_MODE, &polyMode);
    if(glfwGetKey(win, GLFW_KEY_2) && polyModeBT <= currFrame){
        glPolygonMode(GL_FRONT_AND_BACK, polyMode + (polyMode == GL_FILL ? -2 : 1));
        polyModeBT = currFrame + .5f;
    }

    if(glfwGetKey(win, GLFW_KEY_3) && ppeTypeBT <= currFrame){
        typePPE += (typePPE == 5 ? -5 : 1);
        ppeTypeBT = currFrame + .5f;
    }
}

void App::Render(const Cam& cam) const{
    ///1st...: render scenes from lights' POV to depth/shadow map (occluded frags [in shadow], sample closest depth value of each visible frag as seen from light's POV and store them in depth buffer)
    glViewport(0, 0, 1024, 1024); { //Render scene in depth/... map resolution so depth/... map will be neither incomplete nor too small
        //glCullFace(GL_FRONT); { //Solves peter panning (shadow mapping artefact, shadow bias applied leads to visible offset of obj shadows, adjust shadow bias to avoid) for solid objs but causes shadows of plane objs to disappear //Take depth of back faces so shadows will form inside objs??
            RenderSceneToCreatedFB(Cam(glm::vec3(-2.f, 4.f, -1.f), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), 0), dDepthMapFBO, enFBO, 0);
            //RenderSceneToCreatedFB(Cam(cam.GetPos(), cam.GetTarget(), 1), depthMapS, enTex, 0);
        //} glCullFace(GL_BACK); //Another consideration is that objects that are close to the shadow receiver (like the distant cube) may still give incorrect results??
    } glViewport(0, 0, 800, 600); //Render scene in win resolution
    
    ////2nd set of render passes: render scenes with shadow mapping
    uint depthTexs[]{dDepthMapFBO->GetTex().GetRefID(), sDepthMapFBO->GetTex().GetRefID()};

    ///Dynamic environment mapping (use dynamically-generated cubemap textured with 6 diff angles of scene as seen from a cam to create reflective and refractive surfaces that include other instances, avoid and/or use pre-rendered cubemaps as expensive)
    Cam enCam(glm::vec3(0.f, .2f, 0.f), glm::vec3(0.f), glm::vec3(0.f), 2, 1.f);
    float initialAngularFOV = angularFOV;
    angularFOV = 90.f;
    glViewport(0, 0, 1700, 1700);
    for(short i = 0; i < 6; ++i){
        switch(i){
            case 0: enCam.SetTarget(glm::vec3(1.f, 0.f, 0.f)); enCam.SetUp(glm::vec3(0.f, -1.f, 0.f)); break;
            case 1: enCam.SetTarget(glm::vec3(-1.f, 0.f, 0.f)); enCam.SetUp(glm::vec3(0.f, -1.f, 0.f)); break;
            case 2: enCam.SetTarget(glm::vec3(0.f, 1.f, 0.f)); enCam.SetUp(glm::vec3(0.f, 0.f, 1.f)); break;
            case 3: enCam.SetTarget(glm::vec3(0.f, -1.f, 0.f)); enCam.SetUp(glm::vec3(0.f, 0.f, 1.f)); break;
            case 4: enCam.SetTarget(glm::vec3(0.f, 0.f, 1.f)); enCam.SetUp(glm::vec3(0.f, -1.f, 0.f)); break;
            default: enCam.SetTarget(glm::vec3(0.f, 0.f, -1.f)); enCam.SetUp(glm::vec3(0.f, -1.f, 0.f));
        }
        RenderSceneToCreatedFB(enCam, enFBO, 0, depthTexs, i);
    }
    glViewport(0, 0, 800, 600);
    angularFOV = initialAngularFOV;

    RenderSceneToCreatedFB(cam, frontFBO , enFBO, depthTexs);
    RenderSceneToCreatedFB(Cam(cam.GetPos(), cam.GetPos() + cam.GetPos() - cam.GetTarget(), cam.GetUp()), backFBO, enFBO, depthTexs);
    glClear(GL_COLOR_BUFFER_BIT);
    RenderSceneToDefaultFB(frontFBO, intermediateFBO, typePPE);
    //RenderSceneToDefaultFB(backFBO, intermediateFBO, typePPE, glm::vec3(-.5f, .5f, 0.f), glm::vec3(.5f));
}

///Attach a z-buffer/depth buffer (stores depth value of frags clamped to [0, 1] from a cam's POV as 16, 24 or 32 bit floats, same width and height as colour buffer) and a stencil buffer as a single tex (each 32-bit value of the tex contains 24 bits of depth info and 8 bits of stencil info)
//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL); //Config the tex's formats to contain combined depth and stencil values
//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texColorBuffer, 0);

//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
//glBlitFramebuffer(0, 0, 800, 600, 0, 0, 800, 600, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);