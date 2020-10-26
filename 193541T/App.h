#pragma once
#include "Scene.h"
#include "Buffer.h"

class App final{
	float lastFrame;
	float camResetBT;
	float cullBT;
	float polyModeBT;
	float ppeTypeBT;
	int typePPE;
	Scene* scene;
	Framebuffer* frontFBO;
	Framebuffer* backFBO;
	Framebuffer* dDepthMapFBO;
	Framebuffer* sDepthMapFBO;
	Framebuffer* enFBO;
	Framebuffer* intermediateFBO;
	void RenderSceneToCreatedFB(const Cam&, const Framebuffer* const&, const Framebuffer* const&, const uint* const&, const short& = 999) const;
	void RenderSceneToDefaultFB(const Framebuffer* const&, const Framebuffer* const&, const int&, const glm::vec3&& = glm::vec3(0.f), const glm::vec3&& = glm::vec3(1.f)) const;
public:
	App();
	~App();
	static GLFWwindow* win;
	static bool Key(int);
	void Init(), Update(const Cam&);
	void Render(const Cam&) const;
};