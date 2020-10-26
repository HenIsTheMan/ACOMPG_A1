#version 330 core
layout (points) in; //Input primitive type as layout qualifier/...
layout (points, max_vertices = 1) out;
//layout (line_strip, max_vertices = 2) out; //Output primitive type and... as layout qualifiers/... //Line strip binds tgt a set of >= 2 pts to form 1 continuous line
//layout (triangle_strip, max_vertices = 5) out; //The output primitive is rasterized and the fragment shader runs on the whole of it

in myInterface{ //Input interface block
    vec4 Colour;
    vec2 TexCoords;
    vec3 Normal;
    vec3 FragPosWorldSpace;
} gsIn[]; //Interface block arr (arr as most render primitives formed in Primitive/Shape Assembly have > 1 vertex)

out myInterface{
    vec4 Colour; //Fragment shader expects only 1 interpolated colour //Emitted vertices have the last stored value in Colour
    vec2 TexCoords;
    vec3 Normal;
    vec3 FragPosWorldSpace;
} gsOut;

void MakePt(vec4 pos){ //Pass-through geometry shader (takes a primitive as its input and passes it to the next shader unmodified)
    gl_Position = pos;
    EmitVertex();
}

void MakeLine(vec4 pos){
    gl_Position = pos + vec4(-.5f, 0.f, 0.f, 0.f); 
    EmitVertex(); //Adds vec currently set to gl_Position to the output primitive??
    gl_Position = pos + vec4(.5f, 0.f, 0.f, 0.f);
    EmitVertex();
}

void MakeHouse(vec4 pos){ //Generates 5 vertices per pt primitive input to form 1 triangle strip??
    gl_Position = pos + vec4(-.2f, -.2f, 0.f, 0.f);
    EmitVertex();   
    gl_Position = pos + vec4(.2f, -.2f, 0.f, 0.f);
    EmitVertex();
    gl_Position = pos + vec4(-.2f, .2f, 0.f, 0.f);
    EmitVertex();
    gl_Position = pos + vec4(.2f, .2f, 0.f, 0.f);
    EmitVertex();
    gl_Position = pos + vec4(0.f, .4f, 0.f, 0.f);
    gsOut.Colour = vec4(1.f);
    EmitVertex();
}

void main(){
    gsOut.Colour = gsIn[0].Colour;
    gsOut.TexCoords = gsIn[0].TexCoords;
    gsOut.Normal = gsIn[0].Normal;
    gsOut.FragPosWorldSpace = gsIn[0].FragPosWorldSpace;

    MakePt(gl_in[0].gl_Position);
    //MakeLine(gl_in[0].gl_Position);
    //MakeHouse(gl_in[0].gl_Position); //Each house consists of 3 triangles

    EndPrimitive(); //All emitted vertices for the output primitive are combined into the specified output render primitive //Call EndPrimitive() repeatedly after >= 1 EmitVertex() calls to gen multiple primitives
}