#pragma once
#include "Src.h"
#include "Mesh.h"
#include "ShaderProg.h"

class Cam final{
	float aspectRatio;
	glm::vec3 pos, target, up;
	glm::vec3 defaultPos, defaultTarget, defaultUp;
	short projectionType;
public:
	Cam(const glm::vec3&, const glm::vec3&, const glm::vec3&, const short&& = 2, const float&& = 800.f / 600.f);
	glm::vec3 CalcFront(bool = 1) const, CalcRight() const, CalcUp() const;
	glm::mat4 LookAt() const;
	void Update(const int&, const int&, const int&, const int&, const int&, const int&);
	void Reset();

	///Getters
	const float& GetAspectRatio() const;
	const short& GetProjectionType() const;
	const glm::vec3& GetPos() const;
	const glm::vec3& GetTarget() const;
	const glm::vec3& GetUp() const;

	///Setters
	void SetPos(const glm::vec3&);
	void SetTarget(const glm::vec3&);
	void SetUp(const glm::vec3&);
};