#include "Image.hpp"

#include "Device.hpp"

Image::Image(const Device& device, vk::Image image, ImageInfo& imageInfo)
	:device{device}
	,handle(image)
	,imageInfo(imageInfo)
{
}

Image::Image(const Device& device, ImageInfo& imageInfo)
	:device{ device }
	, imageInfo(imageInfo)
{
	vk::ImageCreateInfo createInfo;
	createInfo.imageType = imageInfo.type;
	createInfo.format = imageInfo.format;
	createInfo.extent = imageInfo.extent;
	createInfo.usage = imageInfo.usage;
	createInfo.arrayLayers = imageInfo.arrayLayers;
	createInfo.mipLevels = imageInfo.mipmapLevel;
	createInfo.queueFamilyIndexCount = imageInfo.queueFamilyCount;
	createInfo.pQueueFamilyIndices = imageInfo.pQueueFamilyIndices;
	createInfo.tiling = imageInfo.tiling;
	createInfo.sharingMode = imageInfo.sharingMode;
	createInfo.flags = imageInfo.createFlag;

	auto cCreateInfo = static_cast<VkImageCreateInfo>(createInfo);

	VmaAllocationCreateInfo allocationInfo;
	allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;

	VkImage cHandle;
	VmaAllocation allocation;
	vmaCreateImage(device.getAllocator(), &cCreateInfo, &allocationInfo, &cHandle, &allocation, nullptr);
	
	handle = static_cast<vk::Image>(cHandle);
}

Image::~Image()
{
	if (allocation) {
		vmaDestroyImage(device.getAllocator(), static_cast<VkImage>(handle), allocation);
	}
}

vk::Image Image::getHandle() const
{
	return handle;
}

VmaAllocation Image::getAllocation()
{
	return this->allocation;
}
