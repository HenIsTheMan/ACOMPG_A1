#include "Utility.h"

Vertex::Vertex(const glm::vec3& newPos, const glm::vec4& newColour, const glm::vec2& newTexCoords, const glm::vec3& newNormal, const glm::vec3& newTangent, const glm::vec3& newBitangent):
    pos(newPos),
    colour(newColour),
    texCoords(newTexCoords),
    normal(newNormal),
    tangent(newTangent),
    bitangent(newBitangent){}

PointLight::PointLight(const glm::vec3& pos, const float& constant, const float& linear, const float& quadratic) noexcept{
    this->pos = pos;
    this->constant = constant;
    this->linear = linear;
    this->quadratic = quadratic;
}

DirectionalLight::DirectionalLight(const glm::vec3& dir) noexcept{
    this->dir = dir;
}

Spotlight::Spotlight(const glm::vec3& pos, const glm::vec3& dir, const float& cosInnerCutoff, const float& cosOuterCutoff) noexcept{
    this->pos = pos;
    this->dir = dir;
    this->cosInnerCutoff = cosInnerCutoff;
    this->cosOuterCutoff = cosOuterCutoff;
}