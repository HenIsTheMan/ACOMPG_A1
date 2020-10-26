#pragma once
#include "Src.h"
#include "Utility.h"

class LightChief final{
    LightChief() = default;
    ~LightChief() = default;
    LightChief(LightChief const&) = default;
    LightChief(LightChief&&) = default;
    LightChief& operator=(LightChief const&) = default;
    LightChief& operator=(LightChief&&) = default;
public:
    static std::vector<PointLight> pLights;
    static std::vector<DirectionalLight> dLights;
    static std::vector<Spotlight> sLights;

    static void CreateLightP(const glm::vec3&, const float&, const float&, const float&) noexcept;
    static void CreateLightD(const glm::vec3&) noexcept;
    static void CreateLightS(const glm::vec3&, const glm::vec3&, const float&, const float&) noexcept;
};