#pragma once

#include "Window.hpp"
#include "Core/GPUFramework/GPUContext.hpp"

#include <string>
#include <memory>
#include <vector>

struct ApplicationConfig
{
	std::string name;
	uint32_t width;
	uint32_t height;
	std::vector<const char*> layers;
	std::vector<const char*> extensions;
	uint32_t maxFrames = 2;
};

class Application {
public:
	Application() = default;
	~Application() = default;

	void init(ApplicationConfig& config);

	void run(float deltaTime);

	void close();

protected:
	ApplicationConfig config;

	std::unique_ptr<Window> window;

	std::unique_ptr<GPUContext> gpuContext;

	bool headless = false;

	bool resizable = false;

	bool shouldClose = false;

	bool vsyncOn = false;

	uint32_t fps;

	float frameTime;


private:
	void recordCommandBuffer(vk::CommandBuffer commandBuffer, int imageIndex);

	CommandPool* commandPool;
	RenderPass* renderPass;
	GraphicsPipeline* graphicsPipeline;
	std::vector<Framebuffer*> framebuffers;

	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	std::vector<VkImage> swapChainImages;
	std::vector<std::shared_ptr<ImageView>> swapChainImageViews;
	uint32_t currentFrame = 0;
};