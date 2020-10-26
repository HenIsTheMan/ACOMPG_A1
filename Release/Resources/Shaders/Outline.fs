#version 330 core
out vec4 FragColor;

in myInterface{
    vec4 Colour;
    vec2 TexCoords;
    vec3 Normal;
    vec3 FragPosWorldSpace;
} fsIn;

uniform vec3 myRGB;
uniform float myAlpha;
uniform sampler2D outlineTex;

void main(){
    if(texture(outlineTex, fsIn.TexCoords).a <= .1f){
        discard; //Discard fragments (not stored in the color buffer) over blending them for fully transparent objs so no depth issues
        return;
    }
    FragColor = vec4(myRGB, myAlpha);
	//FragColor = vec4(gl_FragCoord.y > 300 ? myRGB : vec3(1.f) - myRGB, 1.f);
}

//++ post-processing filters like Gaussian Blur??
/*#version 420 core
out vec4 FragColor;

layout (depth_greater) out float gl_FragDepth; //Redeclare gl_FragDepth with depth condition to preserve some early depth testing //++ more depth conditions??

void main(){
    FragColor = vec4(1.f);
    gl_FragDepth = gl_FragCoord.z + .1f;
}*/