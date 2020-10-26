#pragma once
#include "Src.h"

extern bool firstCall;
extern float leftRightMB;
extern float pitch;
extern float yaw;
extern float lastX;
extern float lastY;
extern float SENS;
extern float angularFOV;

void FramebufferSizeCallback(GLFWwindow*, int width, int height){ //Resize callback
    glViewport(0, 0, width, height); //For viewport transform
} //Aspect ratio??

void CursorPosCallback(GLFWwindow*, double xPos, double yPos){
    if(firstCall){
        firstCall = 0;
    } else{ //Add mouse movement offset between last frame and curr frame
        yaw -= (float(xPos) - lastX) * SENS;
        pitch -= (float(yPos) - lastY) * SENS; //??
    }
    lastX = float(xPos);
    lastY = float(yPos);

    ///For geometric intersection testing by ray casting
    //float x = (2.f * float(xPos)) / 800.f - 1.f, y = 1.f - (2.f * float(yPos)) / 600.f; //Viewport coords (viewportWidth to viewportHeight) to NDC (-1 to 1)
    //glm::vec3 rayNDC(x, y, -1.f); //-1.f so ray points forward
    //glm::vec4 rayClipSpace(rayNDC, 1.f); //1.f to form 4D vec
    //glm::vec4 rayViewSpace(glm::inverse(glm::perspective(glm::radians(angularFOV), 800.f / 600.f, .1f, 9999.f)) * rayClipSpace);
    //rayViewSpace = glm::vec4(rayViewSpace.x, rayViewSpace.y, -1.f, 0.f);
    //rayWorldSpace = glm::inverse(cam->LookAt()) * rayViewSpace;
    //rayWorldSpace = glm::normalize(rayWorldSpace);
}

void MouseButtonCallback(GLFWwindow* win, int button, int action, int mods){ //For mouse buttons
    leftRightMB = float(glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) - glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT));
}

void ScrollCallback(GLFWwindow*, double xOffset, double yOffset){
    angularFOV -= float(xOffset) + float(yOffset);
    angularFOV = std::max(1.f, std::min(75.f, angularFOV));
}

void InitGL(GLFWwindow*& win){
    glfwInit();
    //glfwWindowHint(GLFW_SAMPLES, 4); //4 subsamples in a general pattern per set of screen coords of a pixel to determine pixel coverage //Better pixel coverage precision but more performance reduction with more sample pts as they cause size of... buffers to rise by...
    //Super Sample Anti-Aliasing (SSAA, draw more frags, sample pt in the center of each pixel determines if each pixel is influenced by any frag shader or not) temporarily uses a much higher resolution render buffer to render to and the resolution is downsampled back to normal after the scene is rendered
    //Result of Multisample Anti-Aliasing (MSAA) is a framebuffer with higher resolution depth or stencil buffer where primitive edges are smoother
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //For Mac OS X
#endif

    win = glfwCreateWindow(800, 600, "193541T", 0, 0);
    if(win == 0){ //Get a handle to the created window obj
        printf("Failed to create GLFW window\n");
    }
    glfwMakeContextCurrent(win); //Make context of the window the main context on the curr thread
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        printf("Failed to init GLAD\n");
    }

    glfwSetFramebufferSizeCallback(win, &FramebufferSizeCallback);
    glfwSetCursorPosCallback(win, CursorPosCallback);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //Hide and capture mouse cursor
    glfwSetMouseButtonCallback(win, MouseButtonCallback);
    glfwSetScrollCallback(win, ScrollCallback);
    glClearColor(0.2f, 0.3f, 0.3f, 1.f); //State-setting function

    //Stencil buffer usually contains 8 bits per stencil value that amts to 256 diff stencil values per pixel
    //Use stencil buffer operations to write to the stencil buffer when rendering frags (read stencil values in the same or following frame(s) to pass or discard frags based on their stencil value)
    glEnable(GL_STENCIL_TEST); //Discard frags based on frags of other drawn objs in the scene
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); //Frags update stencil buffer with their ref value when... //++params and options??

    glEnable(GL_DEPTH_TEST); //Done in screen space after the frag shader has run and after the stencil test //(pass ? frag is rendered and depth buffer is updated with new depth value : frag is discarded)

    glEnable(GL_BLEND); //Colour resulting from blend eqn replaces prev colour stored in the colour buffer
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //++options??
    //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO); //sets the RGB components as we've set them previously, but only lets the resulting alpha component be influenced by the source's alpha value??
    //glBlendEquation(GL_FUNC_ADD); //Change operator between src and dst part of blend eqn //++other blend eqns??

    glEnable(GL_CULL_FACE); //Specify vertices in CCW winding order so front faces are rendered in CW order while... //Actual winding order is calculated at the rasterization stage after the vertex shader has run //Vertices are then seen from the viewer's POV

    glEnable(GL_PROGRAM_POINT_SIZE);
    //glEnable(GL_MULTISAMPLE); //Enabled by default on most OpenGL drivers //Multisample buffer (stores a given amt of sample pts per pixel) for MSAA //Multisampling algs are implemented in the rasterizer (combination of all algs and processes that transform vertices of primitives into frags, frags are bound by screen resolution unlike vertices so there is almost nvr a 1-on-1 mapping between... and it must decide at what screen coords will each frag of each interpolated vertex end up at) of the OpenGL drivers
    //Poor screen resolution leads to aliasing as the limited amt of screen pixels causes some pixels to not be rendered along an edge of a fig //Colour output is stored directly in the framebuffer if pixel is fully covered and blending is disabled

    //glEnable(GL_FRAMEBUFFER_SRGB); //Colours from sRGB colour space are gamma corrected after each frag shader run before they are stored in colour buffers of all framebuffers
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}