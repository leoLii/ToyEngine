#pragma once

#include "Window.hpp"
#include "Core/GPUFramework/GPUContext.hpp"
#include "Scene/Mesh.hpp"

#include <chrono>
#include <string>
#include <memory>
#include <vector>

class BasePass;

struct ApplicationConfig
{
	std::string name;
	uint32_t width;
	uint32_t height;
	std::vector<const char*> layers;
	std::vector<const char*> extensions;
	uint32_t maxFrames = 2;
};

class Scene;

class Application {
public:
	Application() = default;
	~Application() = default;

	void init(ApplicationConfig& config, Scene* scene);

	void run();

	void close();

protected:
	ApplicationConfig config;

	std::unique_ptr<Window> window;

	std::unique_ptr<GPUContext> gpuContext;

	bool headless = false;

	bool resizable = false;

	bool shouldClose = false;

	bool vsyncOn = false;

	uint32_t fps = 0;

	uint64_t frameIndex = -1;

	float deltaTime = 0.0;

	std::chrono::time_point<std::chrono::system_clock> lastFrameTime;

	vk::Fence fence;

	vk::CommandBuffer commandBuffer;

	BasePass* basePass = nullptr;
	Scene* scene;

	vk::Semaphore imageAvailableSemaphore;

	vk::Semaphore renderFinishedSemaphore;

private:
};