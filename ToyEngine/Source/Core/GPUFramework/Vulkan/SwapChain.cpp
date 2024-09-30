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
	auto surfaceCapabilities = device.getUsingGPU().getSurfaceCapabilitiesKHR(surface);
	auto surfaceFormats = device.getUsingGPU().getSurfaceFormatsKHR(surface);
	auto presentModes = device.getUsingGPU().getSurfacePresentModesKHR(surface);

	format = vk::Format::eB8G8R8A8Srgb;
	extent = surfaceCapabilities.currentExtent;
	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.surface = surface;
	createInfo.minImageCount = surfaceCapabilities.maxImageCount;
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
	device.getHandle().destroySwapchainKHR();
}

const vk::SwapchainKHR Swapchain::getHandle() const {
	return this->handle;
}

std::vector<vk::Image> Swapchain::getSwapChainImages() const {
	return this->images;
}

vk::Format Swapchain::getFormat() {
	return this->format;
}

vk::Extent2D Swapchain::getExtent() {
	return this->extent;
}

