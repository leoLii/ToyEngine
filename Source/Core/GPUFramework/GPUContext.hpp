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

#include "Vulkan/Image.hpp"
#include "Vulkan/ImageView.hpp"
#include "Vulkan/Buffer.hpp"
#include "Vulkan/BufferView.hpp"

#include "Platform/Window.hpp"

#include <vector>
#include <memory>
#include <unordered_map>

/// <summary>
/// GPUContext is designed to deal with physical level functions.
/// </summary>

class GPUContext {
public:
	GPUContext() = delete;

	GPUContext(
		const std::string,
		const std::vector<const char*>& = {}, 
		const std::vector<const char*>& = {}, 
		Window* = nullptr);

	~GPUContext();

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
	vk::CommandBuffer requestCommandBuffer(CommandType, vk::CommandBufferLevel level) const;

	// Pipeline
	PipelineLayout* createPipelineLayout(std::vector<vk::DescriptorSetLayout>, std::vector<vk::PushConstantRange>) const;
	GraphicsPipeline* createGraphicsPipeline(PipelineLayout*, GraphicsPipelineState*, std::vector<const ShaderModule*>) const;


	// Fence & Semaphore
	vk::Fence requestFence() const;

	void waitForFences(const vk::Fence) const;

	void resetFences(const vk::Fence) const;

	void returnFence(const vk::Fence) const;

	vk::Semaphore requestSemaphore() const;

	void returnSemaphore(const vk::Semaphore) const;

	// ShaderModule
	const ShaderModule* findShader(const std::string&) const;

	// Descriptor
	DescriptorSetLayout* createDescriptorSetLayout(uint32_t, std::vector<vk::DescriptorSetLayoutBinding>) const;

	DescriptorSet* requireDescriptorSet(
		DescriptorSetLayout*,
		std::unordered_map<uint32_t, vk::DescriptorBufferInfo>&,
		std::unordered_map<uint32_t, vk::DescriptorImageInfo>&) const;

	// Image & Buffer
	ImageView* createImageView(
		Image*, 
		vk::ImageViewType = vk::ImageViewType::e2D,
		vk::Format = vk::Format::eB8G8R8A8Srgb,
		vk::ComponentMapping = vk::ComponentMapping{},
		vk::ImageSubresourceRange = vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }) const;
	void destroyImageView(ImageView*) const;

	Buffer* createBuffer(uint64_t, vk::BufferUsageFlags) const;
	void destroyBuffer(Buffer*) const;

	Image* createImage(ImageInfo) const;
	void destroyImage(Image*) const;

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
	);

	void transferImage(
		vk::CommandBuffer,
		vk::PipelineStageFlags, vk::PipelineStageFlags,
		vk::AccessFlags, vk::AccessFlags,
		vk::ImageLayout, vk::ImageLayout,
		const Image*, 
		vk::DependencyFlags = vk::DependencyFlagBits::eByRegion,
		vk::ImageSubresourceRange = vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
		uint32_t = VK_QUEUE_FAMILY_IGNORED, uint32_t = VK_QUEUE_FAMILY_IGNORED
		) const;

protected:
	std::vector<const char*> vulkanExtensions;
	std::vector<const char*> vulkanLayers;

	std::unique_ptr<Instance> instance;
	std::unique_ptr<Device> device;
	std::unique_ptr<Swapchain> swapchain;
	std::unique_ptr<SemaphorePool> semaphorePool;
	std::unique_ptr<FencePool> fencePool;
	std::unordered_map<std::string, ShaderModule*> shaderModules;

	vk::DescriptorPool descriptorPool;

	vk::SurfaceKHR surface;

	std::vector<CommandPool*> commandPools;

	std::vector<std::shared_ptr<ImageView>> imageViews;

private:
	void createCommandPools();
	void destroyCommandPools();
	void loadShaders(const std::string& dir);
	void destroyShaders();
	void createSurface(Window*);
	void destroySurface();
	void createDescriptorPool();
	void destroyDescriptorPool();
};