#pragma once
#include "ShaderProg.h"

struct Vertex final{ //Can use to index each vertex attrib
    Vertex(const glm::vec3&, const glm::vec4&, const glm::vec2&, const glm::vec3&, const glm::vec3& = glm::vec3(0.f), const glm::vec3& = glm::vec3(0.f));
    glm::vec3 pos;
    glm::vec4 colour;
    glm::vec2 texCoords;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

struct PointLight final{
    PointLight(const glm::vec3&, const float&, const float&, const float&) noexcept;
    glm::vec3 pos;
    float constant;
    float linear;
    float quadratic;
};

struct DirectionalLight final{
    DirectionalLight(const glm::vec3&) noexcept;
    glm::vec3 dir;
};

struct Spotlight final{
    Spotlight(const glm::vec3&, const glm::vec3&, const float&, const float&) noexcept;
    glm::vec3 pos;
    glm::vec3 dir;
    float cosInnerCutoff; //More efficient than passing in cut-off angle as can directly compare...
    float cosOuterCutoff; //...
};