//
//  SwapChain.cpp
//  ToyEngine
//
//  Created by Li Han on 9/25/24.
//

#include "SwapChain.hpp"
#include "Device.hpp"

Swapchain::Swapchain(const Device& device, const vk::SurfaceKHR surface)
	:device(device)
{
	this->surface = surface;
	surfaceCapabilities = device.getUsingGPU().getSurfaceCapabilitiesKHR(surface);
	surfaceFormats = device.getUsingGPU().getSurfaceFormatsKHR(surface);
	presentModes = device.getUsingGPU().getSurfacePresentModesKHR(surface);

	imageCount = surfaceCapabilities.minImageCount + 1;

	imageInfo.format = vk::Format::eB8G8R8A8Srgb;
	imageInfo.extent = vk::Extent3D(surfaceCapabilities.currentExtent, 1);
	imageInfo.type = vk::ImageType::e2D;
	imageInfo.mipmapLevel = 1;
	imageInfo.tiling = vk::ImageTiling::eLinear;
	imageInfo.arrayLayers = 1;
	imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment;
	imageInfo.sharingMode = vk::SharingMode::eConcurrent;
	imageInfo.queueFamilyCount = 2;
	uint32_t queueFamilyIndices[2] = { 0, 2 };
	imageInfo.pQueueFamilyIndices = queueFamilyIndices;

	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
	createInfo.imageFormat = imageInfo.format;
	createInfo.imageExtent = vk::Extent2D(imageInfo.extent.width, imageInfo.extent.height);
	createInfo.imageArrayLayers = imageInfo.arrayLayers;
	createInfo.imageUsage = imageInfo.usage;
	createInfo.imageSharingMode = imageInfo.sharingMode;
	createInfo.queueFamilyIndexCount = imageInfo.queueFamilyCount;
	createInfo.pQueueFamilyIndices = imageInfo.pQueueFamilyIndices;
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
	auto nativeImages = device.getHandle().getSwapchainImagesKHR(handle);
	for (auto i : nativeImages) {
		auto image = new Image(device, i, imageInfo);
		auto imageView = new ImageView(
			device, image,
			vk::ImageViewType::e2D,
			imageInfo.format,
			vk::ComponentMapping{},
			vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
		images.push_back(image);
		imageViews.push_back(imageView);
	}
}


Swapchain::~Swapchain() {
	for (auto image : images) {
		delete image;
	}
	images.clear();
	for (auto imageView : imageViews) {
		delete imageView;
	}
	imageViews.clear();
	device.getHandle().destroySwapchainKHR(handle);
}

void Swapchain::rebuildWithSize(vk::Extent2D size) {
//	vk::SwapchainCreateInfoKHR createInfo;
//	createInfo.surface = surface;
//	createInfo.minImageCount = imageCount;
//	createInfo.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
//	createInfo.imageFormat = imageInfo.format;
//	createInfo.imageExtent = vk::Extent2D(imageInfo.extent.width, imageInfo.extent.height);
//	createInfo.imageArrayLayers = imageInfo.arrayLayers;
//	createInfo.imageUsage = imageInfo.usage;
//	createInfo.imageSharingMode = imageInfo.sharingMode;
//	createInfo.queueFamilyIndexCount = imageInfo.queueFamilyCount;
//	createInfo.pQueueFamilyIndices = imageInfo.pQueueFamilyIndices;
//	createInfo.preTransform = surfaceCapabilities.currentTransform;
//	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
//	createInfo.presentMode = vk::PresentModeKHR::eMailbox;
//	createInfo.clipped = VK_TRUE;
//	createInfo.oldSwapchain = handle;
//
//#ifdef IMAGE_COMPRESSION
//	vk::ImageCompressionControlEXT compressionControl;
//	compressionControl.flags = requestedCompression;
//	compressionControl.compressionControlPlaneCount = 1;
//	compressionControl.pFixedRateFlags = requestedCompressionFixedRate.data();
//	createInfo.pNext = &compressionControl;
//#endif // IMAGE_COMPRESSION
//
//	handle = device.getHandle().createSwapchainKHR(createInfo);
//	auto nativeImages = device.getHandle().getSwapchainImagesKHR(handle);
//	for (auto i : nativeImages) {
//		images.push_back(Image(device, i, imageInfo));
//		imageViews.push_back(
//			ImageView(device, i, 
//				vk::ImageViewType::e2D, 
//				imageInfo.format, 
//				vk::ComponentMapping{}, 
//				vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }));
//	}
}

vk::SwapchainKHR Swapchain::getHandle() const {
	return this->handle;
}

const std::vector<Image*>& Swapchain::getImages() const {
	return this->images;
}

const std::vector<ImageView*>& Swapchain::getImageViews() const {
	return this->imageViews;
}

ImageInfo Swapchain::getSwapchainImageInfo() const
{
	return imageInfo;
}

uint32_t Swapchain::getImageCount() const
{
	return imageCount;
}
