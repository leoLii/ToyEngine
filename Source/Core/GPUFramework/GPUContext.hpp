#pragma once

#include "Vulkan/VkCommon.hpp"

#include "Vulkan/Instance.hpp"
#include "Vulkan/Device.hpp"
#include "Vulkan/Swapchain.hpp"

#include "Vulkan/Framebuffer.hpp"
#include "Vulkan/RenderPass.hpp"

#include "Vulkan/CommandPool.hpp"
#include "Vulkan/SemaphorePool.hpp"
#include "Vulkan/FencePool.hpp"

#include "Vulkan/ShaderModule.hpp"
#include "Vulkan/DescriptorSetLayout.hpp"
#include "Vulkan/DescriptorSet.hpp"
#include "Vulkan/PipelineLayout.hpp"
#include "Vulkan/GraphicsPipeline.hpp"
#include "Vulkan/ComputePipeline.hpp"

#include "Vulkan/Image.hpp"
#include "Vulkan/ImageView.hpp"
#include "Vulkan/Buffer.hpp"
#include "Vulkan/BufferView.hpp"

#include "Platform/Window.hpp"

#include <vector>
#include <memory>
#include <unordered_map>
#include <array>

/// <summary>
/// GPUContext is designed to deal with physical level functions.
/// </summary>

class GPUContext {
public:
	static GPUContext& GetSingleton() {
		static GPUContext gpuContext{};
		return gpuContext;
	}

	void init(const std::string,
		const std::vector<const char*> & = {},
		const std::vector<const char*> & = {},
		Window* = nullptr);

	const Instance* getInstance() const {
		return instance.get();
	}

	// Device
	const Device* getDevice() const;

	const Device& getDeviceRef() const {
		return *device.get();
	}

	// Swapchain
	const std::vector<Image*>& getSwapchainImages() const;

	const std::vector<ImageView*>& getSwapchainImageViews() const;

	uint32_t getSwapchainImageCount() const;

	vk::Format getSwapchainFormat() const;

	vk::Extent2D getSwapchainExtent() const;

	void rebuildSwapchainWithSize(const vk::Extent2D) const;

	const std::tuple<vk::Result, uint32_t> acquireNextImage(const vk::Semaphore, const vk::Fence, uint32_t timeout = UINT32_MAX) const;

	const Swapchain* getSwapchain() const;

	// Surface

	vk::SurfaceKHR getSurface() const;

	// CommandPool
	vk::CommandBuffer requestCommandBuffer(CommandType, vk::CommandBufferLevel, uint32_t) const;

	vk::CommandPool getCommandPool(uint32_t, QueueType) const;

	vk::CommandPool getTextureLoadPool() const;

	// Pipeline
	PipelineLayout* createPipelineLayout(std::vector<vk::DescriptorSetLayout>, std::vector<vk::PushConstantRange>) const;
	GraphicsPipeline* createGraphicsPipeline(PipelineLayout*, vk::PipelineCache, GraphicsPipelineState*, std::vector<const ShaderModule*>) const;
	ComputePipeline* createComputePipeline(PipelineLayout*, vk::PipelineCache, const ShaderModule*) const;

	// Fence & Semaphore
	vk::Fence requestFence() const;

	void waitForFences(const vk::Fence) const;

	void resetFences(const vk::Fence) const;

	void returnFence(const vk::Fence) const;

	vk::Semaphore requestSemaphore() const;

	void returnSemaphore(const vk::Semaphore) const;

	// ShaderModule
	

	// Descriptor
	DescriptorSetLayout* createDescriptorSetLayout(uint32_t, std::vector<vk::DescriptorSetLayoutBinding>) const;

	DescriptorSet* requireDescriptorSet(
		DescriptorSetLayout*,
		std::unordered_map<uint32_t, vk::DescriptorBufferInfo>,
		std::unordered_map<uint32_t, vk::DescriptorImageInfo>) const;

	// Commands
	void submit(
		CommandType,
		std::vector<vk::Semaphore>,
		std::vector<vk::PipelineStageFlags>,
		std::vector<vk::CommandBuffer>,
		std::vector<vk::Semaphore>,
		vk::Fence fence) const;
	void present(
		uint32_t,
		std::vector<vk::Semaphore>
	) const;

	void pipelineBarrier(
		vk::CommandBuffer,
		vk::PipelineStageFlags, vk::PipelineStageFlags,
		vk::AccessFlags, vk::AccessFlags,
		vk::ImageLayout, vk::ImageLayout,
		const Image*, 
		vk::DependencyFlags = vk::DependencyFlagBits::eByRegion,
		vk::ImageSubresourceRange = vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
		uint32_t = VK_QUEUE_FAMILY_IGNORED, uint32_t = VK_QUEUE_FAMILY_IGNORED
		) const;

	void imageBarrier(
		vk::CommandBuffer,
		vk::PipelineStageFlags2, vk::AccessFlags2, 
		vk::PipelineStageFlags2, vk::AccessFlags2,
		vk::ImageLayout, vk::ImageLayout,
		const Image*,
		vk::DependencyFlags = vk::DependencyFlagBits::eByRegion,
		vk::ImageSubresourceRange = vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
		uint32_t = VK_QUEUE_FAMILY_IGNORED, uint32_t = VK_QUEUE_FAMILY_IGNORED
	) const;

	void bufferBarrier(
		vk::CommandBuffer,
		vk::PipelineStageFlags2, vk::AccessFlags2,
		vk::PipelineStageFlags2, vk::AccessFlags2,
		const Buffer*,
		uint32_t = VK_QUEUE_FAMILY_IGNORED, uint32_t = VK_QUEUE_FAMILY_IGNORED
	) const;

protected:
	GPUContext() = default;

	~GPUContext();

	std::vector<const char*> vulkanExtensions;
	std::vector<const char*> vulkanLayers;

	std::unique_ptr<Instance> instance;
	std::unique_ptr<Device> device;
	std::unique_ptr<Swapchain> swapchain;
	std::unique_ptr<SemaphorePool> semaphorePool;
	std::unique_ptr<FencePool> fencePool;

	vk::DescriptorPool descriptorPool;
	vk::SurfaceKHR surface;
	//std::vector<CommandPool*> commandPools;

	std::vector<std::vector<CommandPool*>> commandPools;

	CommandPool* textureLoadPool;

private:
	void createCommandPools();
	void destroyCommandPools();
	
	void createSurface(Window*);
	void destroySurface();
	void createDescriptorPool();
	void destroyDescriptorPool();
};