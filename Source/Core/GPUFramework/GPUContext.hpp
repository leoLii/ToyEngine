#pragma once

#include "Vulkan/VkCommon.hpp"

#include <vector>
#include <memory>
#include <unordered_map>

class Instance;
class Device;
class ShaderModule;
class Window;
class SemaphorePool;
class FencePool;
class Swapchain;
class ImageView;
//class CommandPool;

/// <summary>
/// GPUContext is designed to deal with physical level functions.
/// </summary>

class GPUContext {
public:
	GPUContext() = delete;

	GPUContext(const std::vector<const char*>& = {}, const std::vector<const char*>& = {}, const std::shared_ptr<Window> = nullptr);

	~GPUContext();

	// Device
	const Device* getDevice() const;

	// Swapchain
	const std::vector<vk::Image>& getSwapchainImages() const;

	const vk::Format getSwapchainFormat() const;

	const vk::Extent2D getSwapchainExtent() const;

	void rebuildSwapchainWithSize(const vk::Extent2D) const;

	const std::tuple<vk::Result, uint32_t> acquireNextImage(const vk::Semaphore, const vk::Fence, uint32_t timeout = UINT32_MAX) const;

	const Swapchain* getSwapchain() const;

	// CommandPool


	// Fence & Semaphore
	const vk::Fence requestFence() const;

	void waitForFences(const vk::Fence) const;

	void resetFences(const vk::Fence) const;

	void returnFence(const vk::Fence) const;

	const vk::Semaphore requestSemaphore() const;

	void returnSemaphore(const vk::Semaphore) const;

	// ShaderModule
	const std::shared_ptr<ShaderModule> findShader(const std::string&) const;

	// Image & Buffer
	const std::shared_ptr<ImageView> createImageView(
		const vk::Image, 
		const vk::ImageViewType = vk::ImageViewType::e2D,
		const vk::Format = vk::Format::eB8G8R8A8Srgb,
		const vk::ComponentMapping = vk::ComponentMapping{},
		const vk::ImageSubresourceRange = vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });

protected:
	std::vector<const char*> vulkanExtensions;
	std::vector<const char*> vulkanLayers;

	std::unique_ptr<Instance> instance;
	std::unique_ptr<Device> device;
	std::shared_ptr<Window> window;
	std::unique_ptr<Swapchain> swapchain;
	std::unique_ptr<SemaphorePool> semaphorePool;
	std::unique_ptr<FencePool> fencePool;
	std::unordered_map<std::string, std::shared_ptr<ShaderModule>> shaderModules;

	//std::vector<std::unique_ptr<CommandPool>> commandPools;

	std::vector<std::shared_ptr<ImageView>> imageViews;

private:
	void loadShaders(const std::string& dir);
};