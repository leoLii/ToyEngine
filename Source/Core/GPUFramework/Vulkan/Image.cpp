#include "Image.hpp"

#include "Device.hpp"

Image::Image(const Device& device, vk::Image image, ImageInfo imageInfo)
	:device{device}
	,handle(image)
	,imageInfo(imageInfo)
{
}

Image::Image(const Device& device, ImageInfo imageInfo)
	:device{ device }
	, imageInfo(imageInfo)
{
	VkImageCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	createInfo.imageType = static_cast<VkImageType>(imageInfo.type);
	createInfo.format = static_cast<VkFormat>(imageInfo.format);
	createInfo.extent = static_cast<VkExtent3D>(imageInfo.extent);
	createInfo.usage = static_cast<VkImageUsageFlags>(imageInfo.usage);
	createInfo.arrayLayers = imageInfo.arrayLayers;
	createInfo.mipLevels = imageInfo.mipmapLevel;
	createInfo.queueFamilyIndexCount = imageInfo.queueFamilyCount;
	createInfo.pQueueFamilyIndices = imageInfo.pQueueFamilyIndices;
	createInfo.tiling = static_cast<VkImageTiling>(imageInfo.tiling);
	createInfo.sharingMode = static_cast<VkSharingMode>(imageInfo.sharingMode);
	createInfo.flags = static_cast<VkImageCreateFlags>(imageInfo.createFlag);
	createInfo.samples = static_cast<VkSampleCountFlagBits>(imageInfo.samples);
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkImage image;
	VmaAllocation allocation;
	auto result = vmaCreateImage(device.getAllocator(), &createInfo, &allocInfo, &image, &allocation, nullptr);
	
	handle = static_cast<vk::Image>(image);
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
