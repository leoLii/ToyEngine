#pragma once

#include "VkCommon.hpp"

class Device;

struct ImageInfo {
	vk::Format format;
	vk::Extent3D extent;
	vk::ImageType type;
	vk::ImageLayout layout;
	uint32_t mipmapLevel;
	uint32_t arrayLayers;
	uint32_t samples;
	vk::ImageCreateFlags createFlag;
	vk::SharingMode sharingMode;
	vk::ImageTiling tiling;
	vk::ImageUsageFlags usage;
	uint32_t queueFamilyCount;
	uint32_t* pQueueFamilyIndices;
};

class Image {
public:
	Image() = delete;
	Image(const Device&, vk::Image, ImageInfo&);
	Image(const Device&, ImageInfo&);

	~Image();

	vk::Image getHandle() const;

	VmaAllocation getAllocation();

protected:
	const Device& device;

	vk::Image handle;

	VmaAllocation allocation = nullptr;

	ImageInfo imageInfo;
};