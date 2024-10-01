//
//  SwapChain.cpp
//  ToyEngine
//
//  Created by Li Han on 9/25/24.
//

#include "SwapChain.hpp"
#include "Device.hpp"

Swapchain::Swapchain(Device& device, vk::SurfaceKHR& surface)
	:device(device),
	surface(surface) 
{
	surfaceCapabilities = device.getUsingGPU().getSurfaceCapabilitiesKHR(surface);
	surfaceFormats = device.getUsingGPU().getSurfaceFormatsKHR(surface);
	presentModes = device.getUsingGPU().getSurfacePresentModesKHR(surface);

	format = vk::Format::eB8G8R8A8Srgb;
	extent = surfaceCapabilities.currentExtent;
	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.surface = surface;
	createInfo.minImageCount = surfaceCapabilities.minImageCount + 1;
	createInfo.imageFormat = format;
	createInfo.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
	createInfo.queueFamilyIndexCount = 2;
	uint32_t queueFamilyIndices[2] = { 0, 2 };
	createInfo.pQueueFamilyIndices = queueFamilyIndices;
	createInfo.preTransform = surfaceCapabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = vk::PresentModeKHR::eMailbox;
	createInfo.clipped = VK_TRUE;

#ifdef IMAGE_COMPRESSION
	vk::ImageCompressionControlEXT compressionControl;
	compressionControl.flags = requestedCompression;
	compressionControl.compressionControlPlaneCount = 1;
	compressionControl.pFixedRateFlags = requestedCompressionFixedRate.data();
	createInfo.pNext = &compressionControl;
#endif // IMAGE_COMPRESSION

	handle = device.getHandle().createSwapchainKHR(createInfo);
	images = device.getHandle().getSwapchainImagesKHR(handle);
}


Swapchain::~Swapchain() {
	device.getHandle().destroySwapchainKHR(handle);
}

void Swapchain::rebuildWithSize(vk::Extent2D size) {
	format = vk::Format::eB8G8R8A8Srgb;
	extent = size;
	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.surface = surface;
	createInfo.minImageCount = surfaceCapabilities.minImageCount + 1;
	createInfo.imageFormat = format;
	createInfo.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
	createInfo.queueFamilyIndexCount = 2;
	uint32_t queueFamilyIndices[2] = { 0, 2 };
	createInfo.pQueueFamilyIndices = queueFamilyIndices;
	createInfo.preTransform = surfaceCapabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = vk::PresentModeKHR::eMailbox;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = handle;

#ifdef IMAGE_COMPRESSION
	vk::ImageCompressionControlEXT compressionControl;
	compressionControl.flags = requestedCompression;
	compressionControl.compressionControlPlaneCount = 1;
	compressionControl.pFixedRateFlags = requestedCompressionFixedRate.data();
	createInfo.pNext = &compressionControl;
#endif // IMAGE_COMPRESSION

	vk::SwapchainKHR newSwapchain = device.getHandle().createSwapchainKHR(createInfo);
	device.getHandle().destroySwapchainKHR(handle);
	handle = newSwapchain;
	images = device.getHandle().getSwapchainImagesKHR(handle);
}

const vk::SwapchainKHR Swapchain::getHandle() const {
	return this->handle;
}

std::vector<vk::Image> Swapchain::getSwapchainImages() const {
	return this->images;
}

vk::Format Swapchain::getFormat() {
	return this->format;
}

vk::Extent2D Swapchain::getExtent() {
	return this->extent;
}

