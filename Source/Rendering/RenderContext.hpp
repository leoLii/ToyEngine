#pragma once

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

#include "Core/GPUFramework/Vulkan/VkCommon.hpp"

#include <array>
#include <mutex>

class GPUContext;
class ResourceManager;
class Scene;
class GBufferPass;
class LightingPass;
class TaaPass;
class FrustumCullPass;

struct FrameData {
	std::atomic<bool> readyForRender{ false };  // ֡�����Ƿ�ɹ���Ⱦ�߳�ʹ��
	std::atomic<bool> rendered{ true };
	vk::Semaphore renderFinished, imageAvailable, computeFinished;
	vk::Fence renderFence;
	vk::Fence computeFence;
	vk::CommandBuffer renderCommandBuffer;
	vk::CommandBuffer computeCommandBuffer;
};

class RenderContext {
public:
	static RenderContext& GetSingleton() {
		static RenderContext instance{};
		return instance;
	}

	void prepare(const Scene*);
	void render(uint64_t);
	void clear();

	FrameData& getFrameData(uint64_t);

protected:
	const GPUContext& gpuContext;
	ResourceManager& resourceManager;

	//vk::Fence fence;

	//vk::Semaphore imageAvailableSemaphore;
	//vk::Semaphore renderFinishedSemaphore;
	//vk::Semaphore transferFinishedSemaphore;

	//vk::CommandBuffer renderCommandBuffer;
	//vk::CommandBuffer transferCommandBuffer;

	GBufferPass* gBufferPass = nullptr;
	LightingPass* lightingPass = nullptr;
	TaaPass* taaPass = nullptr;
	FrustumCullPass* cullPass = nullptr;

	std::array<FrameData, 2> frameDatas;

private:
	RenderContext();
	~RenderContext();
	void createAttachments(uint32_t, uint32_t);
};