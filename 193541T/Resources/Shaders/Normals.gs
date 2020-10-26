#version 330 core
layout (triangles) in; //Takes set of vertices that form a primitive as input (so its input data from the vertex shader is always represented as arrays of vertex data even though we only have a single vertex right now??)
layout (line_strip, max_vertices = 6) out; //Shapes are dynamically generated on the GPU with geometry shaders (better than defining shapes within vertex buffers) so good for simple repeating forms like cubes in a voxel world or grass in a field

in myInterface{
    vec4 Colour;
    vec2 TexCoords;
    vec3 Normal;
    vec3 FragPosWorldSpace;
} gsIn[];

out myInterface{
    vec4 Colour;
    vec2 TexCoords;
    vec3 Normal;
    vec3 FragPosWorldSpace;
} gsOut;

uniform float len;

void GenLine(int index){
    for(float i = 0.f; i <= len; i += len){
        gl_Position = gl_in[index].gl_Position + i * vec4(normalize(gsIn[index].Normal), 0.f);
        gsOut.Colour = gsIn[index].Colour;
        gsOut.TexCoords = gsIn[index].TexCoords;
        gsOut.Normal = gsIn[index].Normal;
        gsOut.FragPosWorldSpace = gsIn[index].FragPosWorldSpace;
        EmitVertex();
    }
    EndPrimitive();
}

void main(){
    for(int i = 0; i < 3; ++i){ 
        GenLine(i);
    }
}