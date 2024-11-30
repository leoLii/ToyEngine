#pragma once

#include "Core/GPUFramework/Vulkan/VkCommon.hpp"

class GPUContext;
class ResourceManager;
class Scene;
class GBufferPass;
class LightingPass;
class TaaPass;
class FrustumCullPass;

class RenderContext {
public:
	static RenderContext& GetSingleton() {
		static RenderContext instance{};
		return instance;
	}

	void prepare(const Scene*);
	void render(uint64_t);
	void clear();

protected:
	const GPUContext& gpuContext;
	ResourceManager& resourceManager;

	vk::Fence fence;

	vk::Semaphore imageAvailableSemaphore;
	vk::Semaphore renderFinishedSemaphore;
	vk::Semaphore transferFinishedSemaphore;

	vk::CommandBuffer renderCommandBuffer;
	vk::CommandBuffer transferCommandBuffer;

	GBufferPass* gBufferPass = nullptr;
	LightingPass* lightingPass = nullptr;
	TaaPass* taaPass = nullptr;
	FrustumCullPass* cullPass = nullptr;

private:
	RenderContext();
	~RenderContext();
	void createAttachments(uint32_t, uint32_t);
};