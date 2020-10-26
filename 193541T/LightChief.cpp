#include "LightChief.h"

std::vector<PointLight> LightChief::pLights;
std::vector<DirectionalLight> LightChief::dLights;
std::vector<Spotlight> LightChief::sLights;

void LightChief::CreateLightP(const glm::vec3& pos, const float& constant, const float& linear, const float& quadratic) noexcept{
    pLights.emplace_back(PointLight(pos, constant, linear, quadratic));
}

void LightChief::CreateLightD(const glm::vec3& dir) noexcept{
    dLights.emplace_back(DirectionalLight(dir));
}

void LightChief::CreateLightS(const glm::vec3& pos, const glm::vec3& dir, const float& cosInnerCutoff, const float& cosOuterCutoff) noexcept{
    sLights.emplace_back(Spotlight(pos, dir, cosInnerCutoff, cosOuterCutoff));
}

//++lightColour for all components??