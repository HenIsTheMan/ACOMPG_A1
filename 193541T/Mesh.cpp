#include "Mesh.h"

int Mesh::primitiveRestartIndex = 0;
std::vector<unsigned char> Mesh::heightMap;

Mesh::Mesh(const std::vector<Vertex>* const& vertices, const std::vector<uint>* const& indices) noexcept{
    VAO = VBO = EBO = 0;
    if(vertices){
        this->vertices = *vertices;
    }
    this->indices = const_cast<std::vector<uint>* const&>(indices);
    modelMatrices = nullptr;
}

Mesh::Mesh(const Mesh& other) noexcept{
    VAO = other.VAO;
    VBO = other.VBO;
    EBO = other.EBO;
    vertices = other.vertices;
    if(this != &other){
        indices = new std::vector<uint>(*(other.indices));
    }
    textures = other.textures;
    modelMatrices = nullptr;
}

Mesh::~Mesh() noexcept{
    delete indices;
    glDeleteBuffers(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Mesh::Init(const uint& instanceAmt){
    glGenVertexArrays(1, &VAO); //Gen VAO and get ref ID of it
    glGenBuffers(1, &VBO); //A buffer manages a certain piece of GPU mem
    glGenBuffers(1, &EBO);

    ///Batch vertex vec data into large chunks per vertex attrib type since loading them from file(s) generally gives an arr per...
    std::vector<glm::vec3> pos;
    std::vector<glm::vec4> colour;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normal;
    std::vector<glm::vec3> tangent;
    std::vector<glm::vec3> bitangent;
    for(uint i = 0; i < vertices.size(); ++i){
        pos.emplace_back(vertices[i].pos);
        colour.emplace_back(vertices[i].colour);
        texCoords.emplace_back(vertices[i].texCoords);
        normal.emplace_back(vertices[i].normal);
        tangent.emplace_back(vertices[i].tangent);
        bitangent.emplace_back(vertices[i].bitangent);
    }

    ///Can read data from file to save mem
    size_t size[8]{0, }, biggerSize[8]{0, };
    size[1] = pos.size() * sizeof(glm::vec3);
    size[2] = colour.size() * sizeof(glm::vec4);
    size[3] = texCoords.size() * sizeof(glm::vec2);
    size[4] = normal.size() * sizeof(glm::vec3);
    size[5] = sizeof(translations);
    size[6] = instanceAmt * sizeof(glm::mat4);
    size[7] = tangent.size() * sizeof(glm::vec3);
    for(short i = 1; i < sizeof(biggerSize) / sizeof(biggerSize[0]); ++i){
        biggerSize[i] = biggerSize[i - 1] + size[i];
    }

    int index = 0;
    for(float x = -25.f; x < 25.f; x += 5.f){
        for(float y = -25.f; y < 25.f; y += 5.f){
            translations[index++] = glm::vec2(x, y);
        }
    }

    modelMatrices = new glm::mat4[instanceAmt];
    for(uint i = 0; i < instanceAmt; ++i){
        const float xPos = float(rand() % (501 - 4)) - 248.f;
        const float zPos = float(rand() % (501 - 4)) - 248.f;
        glm::mat4 model = translate(glm::mat4(1.f), glm::vec3(xPos, -100.f + 100.f * Mesh::ReadHeightMap(Mesh::heightMap, xPos / 500.f, zPos / 500.f) + 4.f, zPos));
        model = glm::scale(model, glm::vec3(4.f));
        modelMatrices[i] = model;
    }

    glBindVertexArray(VAO); {
        glBindBuffer(GL_ARRAY_BUFFER, VBO); //Makes VBO the buffer currently bound to the GL_ARRAY_BUFFER target, GL_ARRAY_BUFFER is VBO's type
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex) + size[5] + size[6], NULL, GL_STATIC_DRAW); //Alloc mem with glBufferData 1st //Can combine vertex attrib data into 1 arr or vec and fill VBO's mem with glBufferData
        glBufferSubData(GL_ARRAY_BUFFER, biggerSize[0], size[1], &pos[0]); //Mem layout of all structs in C++ is sequential so can put &pos[0]
        glBufferSubData(GL_ARRAY_BUFFER, biggerSize[1], size[2], &colour[0]); //Range of [biggerSize[1], size[2]]
        glBufferSubData(GL_ARRAY_BUFFER, biggerSize[2], size[3], &texCoords[0]);
        glBufferSubData(GL_ARRAY_BUFFER, biggerSize[3], size[4], &normal[0]);
        uint tempBO;
        glGenBuffers(1, &tempBO);
        glBindBuffer(GL_COPY_READ_BUFFER, tempBO);
        glBufferData(GL_COPY_READ_BUFFER, size[5], translations, GL_STATIC_DRAW);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, biggerSize[4], size[5]); //Copy data of a specified size from 1 buffer to another (from a specified read offset to a specified write offset) //Use GL_COPY_READ_BUFFER and/or... if 2 buffers of the same type
        glBufferSubData(GL_ARRAY_BUFFER, biggerSize[5], size[6], &modelMatrices[0]);
        glBufferSubData(GL_ARRAY_BUFFER, biggerSize[6], size[7], &tangent[0]);
        glBufferSubData(GL_ARRAY_BUFFER, biggerSize[7], bitangent.size() * sizeof(glm::vec3), &bitangent[0]);

        if(indices){
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); //Element index buffer
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, (*indices).size() * sizeof(uint), &((*indices)[0]), GL_STATIC_DRAW); //Allocates a piece of GPU mem and adds data into it (reserve mem only if pass NULL as its data argument)
        }

        //Specify vertex shader layout via vertex attrib pointers (attribs: pos, colour, normal, texture)
        int componentsAmt[]{3, 4, 2, 3, 2, 4, 4, 4, 4, 3, 3};
        for(int i = 0; i < sizeof(componentsAmt) / sizeof(componentsAmt[0]); ++i){
            glEnableVertexAttribArray(i);
            if(i < 5 || i > 8){
                glVertexAttribPointer(i, componentsAmt[i], GL_FLOAT, GL_FALSE, componentsAmt[i] * sizeof(float), (void*)(biggerSize[i])); //Specify a batch (112233) vertex attrib layout for the vertex arr's vertex buffer content
            } else{ //Max amt of data allowed for a vertex attrib is 4 floats so reserve 4 vertex attribs for mat4
                glVertexAttribPointer(i, componentsAmt[i], GL_FLOAT, GL_FALSE, componentsAmt[i] * sizeof(glm::vec4), (void*)(biggerSize[5] + ((size_t)i - 5) * sizeof(glm::vec4)));
            }
        }
        for(short i = 4; i < 9; ++i){
            glVertexAttribDivisor(i, 1); //Params: vertex attrib location, attrib divisor (0 means update vertex attrib to the next element per vertex and non-0 means... every non-0 amt of instances) //Config vertex attrib as an instanced arr (allows for passing more data [limited by mem] to the vertex shader for instanced drawing/...)
        }
    } glBindVertexArray(0); //Break the existing vertex arr obj binding
    delete[] modelMatrices;
}

Mesh* const Mesh::CreatePts(){
    Mesh* mesh = new Mesh;
    mesh->vertices.emplace_back(Vertex(glm::vec3(-1.f, 1.f, 0.f), glm::vec4(1.f, 0.f, 0.f, 1.f), glm::vec2(0.f), glm::vec3(0.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(1.f, 1.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 1.f), glm::vec2(0.f), glm::vec3(0.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(1.f, -1.f, 0.f), glm::vec4(0.f, 0.f, 1.f, 1.f), glm::vec2(0.f), glm::vec3(0.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(-1.f, -1.f, 0.f), glm::vec4(1.f, 1.f, 0.f, 1.f), glm::vec2(0.f), glm::vec3(0.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(0.f, 0.f, 0.f), glm::vec4(1.f, 0.f, 1.f, 1.f), glm::vec2(0.f), glm::vec3(0.f)));
    return mesh;
}

Mesh* const Mesh::CreateQuad(){
    Mesh* mesh = new Mesh;
    glm::vec3 pos0(-1.f, -1.f, 0.f);
    glm::vec3 pos1(1.f, 1.f, 0.f);
    glm::vec3 pos2(-1.f, 1.f, 0.f);
    glm::vec3 pos3(1.f, -1.f, 0.f);
    glm::vec2 uv0(0.f, 0.f);
    glm::vec2 uv1(1.f, 1.f);
    glm::vec2 uv2(0.f, 1.f);
    glm::vec2 uv3(1.f, 0.f);

    glm::vec3 edge1 = pos1 - pos0;
    glm::vec3 edge2 = pos2 - pos0;
    glm::vec2 deltaUV1 = uv1 - uv0;
    glm::vec2 deltaUV2 = uv2 - uv0;
    float f = 1.f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    glm::vec3 tangent1, bitangent1; //For each vertex //T and B lie on the same plane as normal map surface and align with tex axes U and V so calc them with vertices (to get edges of...) and texCoords (since in tangent space) of primitives
    tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    tangent1 = glm::normalize(tangent1);

    bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    bitangent1 = glm::normalize(bitangent1);

    mesh->vertices.emplace_back(Vertex(glm::vec3(-1.f, -1.f, 0.f), glm::vec4(0.f, 0.f, 1.f, 1.f), glm::vec2(0.f, 0.f), glm::vec3(0.f, 0.f, 1.f), tangent1, bitangent1));
    mesh->vertices.emplace_back(Vertex(glm::vec3(1.f, 1.f, 0.f), glm::vec4(0.f, 1.f, 1.f, 1.f), glm::vec2(1.f, 1.f), glm::vec3(0.f, 0.f, 1.f), tangent1, bitangent1));
    mesh->vertices.emplace_back(Vertex(glm::vec3(-1.f, 1.f, 0.f), glm::vec4(1.f, 0.f, 0.f, 1.f), glm::vec2(0.f, 1.f), glm::vec3(0.f, 0.f, 1.f), tangent1, bitangent1));
    mesh->vertices.emplace_back(Vertex(glm::vec3(1.f, -1.f, 0.f), glm::vec4(0.f, 1.f, 0.f, 1.f), glm::vec2(1.f, 0.f), glm::vec3(0.f, 0.f, 1.f), tangent1, bitangent1));
    mesh->indices = new std::vector<uint>{0, 1, 2, 0, 3, 1};
    return mesh;
}

Mesh* const Mesh::CreateCube(){
    Mesh* mesh = new Mesh;

    mesh->vertices.emplace_back(Vertex(glm::vec3(1.f, 1.f, 1.f), glm::vec4(1.f), glm::vec2(1.f, 0.f), glm::vec3(0.f, 1.f, 0.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(1.f, 1.f, -1.f), glm::vec4(1.f), glm::vec2(1.f, 1.f), glm::vec3(0.f, 1.f, 0.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(-1.f, 1.f, -1.f), glm::vec4(1.f), glm::vec2(0.f, 1.f), glm::vec3(0.f, 1.f, 0.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(-1.f, 1.f, 1.f), glm::vec4(1.f), glm::vec2(0.f, 0.f), glm::vec3(0.f, 1.f, 0.f)));

    mesh->vertices.emplace_back(Vertex(glm::vec3(1.f, -1.f, 1.f), glm::vec4(1.f), glm::vec2(0.f, 0.f), glm::vec3(1.f, 0.f, 0.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(1.f, -1.f, -1.f), glm::vec4(1.f), glm::vec2(1.f, 0.f), glm::vec3(1.f, 0.f, 0.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(1.f, 1.f, -1.f), glm::vec4(1.f), glm::vec2(1.f, 1.f), glm::vec3(1.f, 0.f, 0.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(1.f, 1.f, 1.f), glm::vec4(1.f), glm::vec2(0.f, 1.f), glm::vec3(1.f, 0.f, 0.f)));

    mesh->vertices.emplace_back(Vertex(glm::vec3(1.f, 1.f, 1.f), glm::vec4(1.f), glm::vec2(1.f, 1.f), glm::vec3(0.f, 0.f, 1.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(-1.f, 1.f, 1.f), glm::vec4(1.f), glm::vec2(0.f, 1.f), glm::vec3(0.f, 0.f, 1.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(-1.f, -1.f, 1.f), glm::vec4(1.f), glm::vec2(0.f, 0.f), glm::vec3(0.f, 0.f, 1.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(1.f, -1.f, 1.f), glm::vec4(1.f), glm::vec2(1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)));

    mesh->vertices.emplace_back(Vertex(glm::vec3(1.f, -1.f, -1.f), glm::vec4(1.f), glm::vec2(1.f, 0.f), glm::vec3(0.f, 0.f, -1.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(-1.f, -1.f, -1.f), glm::vec4(1.f), glm::vec2(0.f, 0.f), glm::vec3(0.f, 0.f, -1.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(-1.f, 1.f, -1.f), glm::vec4(1.f), glm::vec2(0.f, 1.f), glm::vec3(0.f, 0.f, -1.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(1.f, 1.f, -1.f), glm::vec4(1.f), glm::vec2(1.f, 1.f), glm::vec3(0.f, 0.f, -1.f)));

    mesh->vertices.emplace_back(Vertex(glm::vec3(-1.f, -1.f, -1.f), glm::vec4(1.f), glm::vec2(1.f, 0.f), glm::vec3(-1.f, 0.f, 0.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(-1.f, -1.f, 1.f), glm::vec4(1.f), glm::vec2(0.f, 0.f), glm::vec3(-1.f, 0.f, 0.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(-1.f, 1.f, 1.f), glm::vec4(1.f), glm::vec2(0.f, 1.f), glm::vec3(-1.f, 0.f, 0.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(-1.f, 1.f, -1.f), glm::vec4(1.f), glm::vec2(1.f, 1.f), glm::vec3(-1.f, 0.f, 0.f)));

    mesh->vertices.emplace_back(Vertex(glm::vec3(-1.f, -1.f, -1.f), glm::vec4(1.f), glm::vec2(0.f, 1.f), glm::vec3(0.f, -1.f, 0.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(1.f, -1.f, -1.f), glm::vec4(1.f), glm::vec2(1.f, 1.f), glm::vec3(0.f, -1.f, 0.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(1.f, -1.f, 1.f), glm::vec4(1.f), glm::vec2(1.f, 0.f), glm::vec3(0.f, -1.f, 0.f)));
    mesh->vertices.emplace_back(Vertex(glm::vec3(-1.f, -1.f, 1.f), glm::vec4(1.f), glm::vec2(0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)));

    mesh->indices = new std::vector<uint>;
    short myArr[6]{0, 1, 2, 2, 3, 0};
    for(short i = 0; i < 6; ++i){
        for(short j = 0; j < 6; ++j){
            (*(mesh->indices)).emplace_back(i * 4 + myArr[j]);
        }
    }

    return mesh;
}

Mesh* const Mesh::CreateSlicedTexQuad(const float& quadSize, const float& hTile, const float& vTile){
    Mesh* mesh = new Mesh;
    for(uint z = 0; z < (uint)quadSize; ++z){
        for(uint x = 0; x < (uint)quadSize; ++x){
            mesh->vertices.emplace_back(Vertex({float(x) / quadSize - .5f, 0.f, float(z) / quadSize - .5f}, glm::vec4(1.f), {float(x) / quadSize * hTile, 1.f - float(z) / quadSize * vTile}, {0.f, 1.f, 0.f}));
        }
    }
    mesh->indices = new std::vector<uint>;
    for(uint z = 0; z < uint(quadSize - 1.f); ++z){
        for(uint x = 0; x < uint(quadSize - 1.f); ++x){
            ///Triangle 1
            (*(mesh->indices)).emplace_back(uint(quadSize * z + x + 0));
            (*(mesh->indices)).emplace_back(uint(quadSize * (z + 1) + x + 0));
            (*(mesh->indices)).emplace_back(uint(quadSize * z + x + 1));

            ///Triangle 2
            (*(mesh->indices)).emplace_back(uint(quadSize * (z + 1) + x + 1));
            (*(mesh->indices)).emplace_back(uint(quadSize * z + x + 1));
            (*(mesh->indices)).emplace_back(uint(quadSize * (z + 1) + x + 0));
        }
    }
    return mesh;
}

void Mesh::Draw(const int& primitive, const uint& instanceAmt){
    try{
        if(instanceAmt == 0){
            throw std::runtime_error("Invalid amt of instances!\n");
        }
    } catch(std::runtime_error err){
        printf(err.what());
    }
    if(!VAO && !VBO && !EBO){ //Init on 1st draw/...
        Init(instanceAmt);
    }
    for(uint i = 0; i < textures.size(); ++i){
        if(textures[i].GetActiveOnMesh()){
            ShaderProg::UseTex(GL_TEXTURE_2D, textures[i], ("material." + textures[i].GetType() + "Map").c_str());
        }
    }
    if(primitive == GL_TRIANGLE_STRIP){
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(primitiveRestartIndex);
    }
    glBindVertexArray(VAO); {
        if(instanceAmt == 1){
            indices ? glDrawElements(primitive, GLsizei((*indices).size()), GL_UNSIGNED_INT, 0) : glDrawArrays(primitive, 0, (GLsizei)vertices.size()); //Draw/Render call/command
        } else{
            indices ? glDrawElementsInstanced(primitive, GLsizei((*indices).size()), GL_UNSIGNED_INT, 0, instanceAmt) : glDrawArraysInstanced(primitive, 0, (GLsizei)vertices.size(), instanceAmt); //For instanced drawing/rendering (Draw/... objs with equal mesh data in 1 draw/... call so save on CPU-to-GPU comms [slow])
        }
    } glBindVertexArray(0); //Break the existing VAO binding
    if(primitive == GL_TRIANGLE_STRIP){
        glDisable(GL_PRIMITIVE_RESTART);
    }
    for(uint i = 0; i < textures.size(); ++i){
        if(textures[i].GetActiveOnMesh()){
            ShaderProg::StopUsingTex(GL_TEXTURE_2D, textures[i]);
            textures[i].SetActiveOnMesh(0);
        }
    }
}

void Mesh::LoadTex(const cstr& fPath, const str& type, const bool&& flipTex){
    Tex tex;
    const std::vector<cstr>* const fPaths = new std::vector<cstr>{fPath};
    tex.Create(GL_TEXTURE_2D, 999, 0, 0, GL_NEAREST, GL_LINEAR, GL_REPEAT, type, fPaths, flipTex);
    delete fPaths;
    textures.emplace_back(tex);
}

bool Mesh::LoadHeightMap(cstr file_path, std::vector<unsigned char>& heightMap){
    std::ifstream fileStream(file_path, std::ios::binary);
    if(!fileStream.is_open()){
        std::cout << "Impossible to open " << file_path << ". Are you in the right directory ?\n";
        return false;
    }

    fileStream.seekg(0, std::ios::end);
    std::streampos fsize = (unsigned)fileStream.tellg();

    fileStream.seekg(0, std::ios::beg);
    heightMap.resize((unsigned)fsize);
    fileStream.read((char*)&heightMap[0], fsize);

    fileStream.close();
    return true;
}

float Mesh::ReadHeightMap(std::vector<unsigned char> &heightMap, float x, float z){
    if(heightMap.size() == 0 || x <= -0.5f || x >= 0.5f || z <= -0.5f || z >= 0.5f){
        return 0.f;
    }

    const float SCALE_FACTOR = 256.f;
    unsigned terrainSize = (unsigned)sqrt((double)heightMap.size());
    unsigned zCoord = (unsigned)((z + 0.5f) * terrainSize);
    unsigned xCoord = (unsigned)((x + 0.5f) * terrainSize);

    return (float)heightMap[zCoord * terrainSize + xCoord] / SCALE_FACTOR;
}

Mesh* const Mesh::CreateHeightMap(cstr const& fPath, const float& hTile, const float& vTile){
    Mesh* mesh = new Mesh;
    if(!LoadHeightMap(fPath, heightMap)){
        exit(1);
    }
    const float SCALE_FACTOR = 256.f; //Set a scale factor to size terrain
    unsigned terrainSize = (unsigned)sqrt((double)heightMap.size()); //Calc the terrainSize

    std::vector<std::vector<glm::vec3>> pos = std::vector<std::vector<glm::vec3>>(terrainSize, std::vector<glm::vec3>(terrainSize));
    for(unsigned z = 0; z < terrainSize; ++z){
        for(unsigned x = 0; x < terrainSize; ++x){
            float scaledHeight = (float)heightMap[z * terrainSize + x] / SCALE_FACTOR; //Divide by SCALE_FACTOR to normalise
            pos[z][x] = glm::vec3(float(x) / terrainSize - .5f, scaledHeight, float(z) / terrainSize - .5f);
        }
    }

    std::vector<std::vector<glm::vec3>> normals = std::vector<std::vector<glm::vec3>>(terrainSize, std::vector<glm::vec3>(terrainSize));
    std::vector<std::vector<glm::vec3>> tempNormals[2];
    for(short i = 0; i < 2; ++i){
        tempNormals[i] = std::vector<std::vector<glm::vec3>>(terrainSize, std::vector<glm::vec3>(terrainSize));
    }
    for(unsigned z = 0; z < terrainSize - 1; ++z){
        for(unsigned x = 0; x < terrainSize - 1; ++x){
            const auto& vertexA = pos[z][x];
            const auto& vertexB = pos[z][x + 1];
            const auto& vertexC = pos[z + 1][x + 1];
            const auto& vertexD = pos[z + 1][x];
            const auto triangleNormalA = glm::cross(vertexB - vertexA, vertexA - vertexD);
            const auto triangleNormalB = glm::cross(vertexD - vertexC, vertexC - vertexB);
            tempNormals[0][z][x] = triangleNormalA.length() ? glm::normalize(triangleNormalA) : triangleNormalA;
            tempNormals[1][z][x] = triangleNormalB.length() ? glm::normalize(triangleNormalB) : triangleNormalB;
        }
    }
    for(unsigned z = 0; z < terrainSize; ++z){
        for(unsigned x = 0; x < terrainSize; ++x){
            const auto isFirstRow = z == 0;
            const auto isFirstColumn = x == 0;
            const auto isLastRow = z == terrainSize - 1;
            const auto isLastColumn = x == terrainSize - 1;
            auto finalVertexNormal = glm::vec3(0.f);
            if(!isFirstRow && !isFirstColumn){ //Look for triangle to the upper-left
                finalVertexNormal += tempNormals[0][z - 1][x - 1];
            }
            if(!isFirstRow && !isLastColumn){ //Look for triangles to the upper-right
                for(auto k = 0; k < 2; ++k){
                    finalVertexNormal += tempNormals[k][z - 1][x];
                }
            }
            if(!isLastRow && !isLastColumn){ //Look for triangle to the bottom-right
                finalVertexNormal += tempNormals[0][z][x];
            }
            if(!isLastRow && !isFirstColumn){ //Look for triangles to the bottom-right
                for(auto k = 0; k < 2; ++k){
                    finalVertexNormal += tempNormals[k][z][x - 1];
                }
            }
            normals[z][x] = finalVertexNormal.length() ? glm::normalize(finalVertexNormal) : finalVertexNormal; //Store final normal of j-th vertex in i-th row //Normalize to give avg of 4 normals
            mesh->vertices.emplace_back(Vertex(pos[z][x], glm::vec4(1.f), glm::vec2(float(x) / terrainSize * hTile, 1.f - float(z) / terrainSize * vTile), normals[z][x]));
        }
    }

    mesh->indices = new std::vector<uint>;
    for(unsigned z = 0; z < terrainSize - 1; ++z){
        for(unsigned x = 0; x < terrainSize - 1; ++x){
            ///Triangle 1
            mesh->indices->emplace_back(terrainSize * z + x + 0);
            mesh->indices->emplace_back(terrainSize * (z + 1) + x + 0);
            mesh->indices->emplace_back(terrainSize * z + x + 1);

            ///Triangle 2
            mesh->indices->emplace_back(terrainSize * (z + 1) + x + 1);
            mesh->indices->emplace_back(terrainSize * z + x + 1);
            mesh->indices->emplace_back(terrainSize * (z + 1) + x + 0);
        }
    }
    return mesh;
}

//uint* indicesPtr = new uint[indices.size()];
//std::copy(indices.begin(), indices.end(), indicesPtr);
//void* ptr = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY); //Get ptr to currently bound buffer's mem
//memcpy(ptr, (void*)indicesPtr, indices.size() * sizeof(uint)); //Copy data into the mem
//glUnmapNamedBuffer(EBO); //The ptr becomes invalid and the function returns GL_TRUE if OpenGL was able to map your data successfully to the buffer??

//glVertexAttribPointer(i, componentsAmt[i], GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)((!i ? 0 : std::accumulate(componentsAmt, componentsAmt + i, 0)) * floatSize)); //Specify an interleaved (123123, diff vertex attrib data next to one another in mem per vertex) attrib layout for the vertex array buffer's content
//GLsizei(std::accumulate(componentsAmt, componentsAmt + arrSize, 0) * floatSize)
//glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)); //The macro returns the byte offset of that variable from the start of the struct