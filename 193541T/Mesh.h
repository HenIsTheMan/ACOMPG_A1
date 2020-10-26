#pragma once
#include "Buffer.h"
#include "Src.h"
#include "Utility.h"

class Mesh{ //Single drawable entity
    glm::vec2 translations[100];
    glm::mat4* modelMatrices;
protected:
    uint VAO, VBO, EBO;
    void Init(const uint&);
public:
    Mesh(const std::vector<Vertex>* const& = nullptr, const std::vector<uint>* const& = nullptr) noexcept;
    Mesh(const Mesh&) noexcept; //Avoid shallow copy so won't delete a wild ptr
    virtual ~Mesh() noexcept;

    ///In local space (local to mesh)
    static Mesh* const CreatePts();
    static Mesh* const CreateQuad();
    static Mesh* const CreateCube();
    static Mesh* const CreateSlicedTexQuad(const float&, const float&, const float&);

    std::vector<Vertex> vertices;
    std::vector<uint>* indices;
    std::vector<Tex> textures;
    void Draw(const int&, const uint&);
    void LoadTex(const cstr&, const str&, const bool&& = 1);

    static int primitiveRestartIndex;
    static std::vector<unsigned char> heightMap;
    static bool LoadHeightMap(cstr, std::vector<unsigned char>&);
    static float ReadHeightMap(std::vector<unsigned char> &heightMap, float x, float z);
    static Mesh* const CreateHeightMap(cstr const&, const float& hTile, const float& vTile);
};