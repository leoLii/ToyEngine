#pragma once

#include "VkCommon.hpp"

class Device;
class ImageView;

struct ImageInfo {
	vk::Format format;
	vk::Extent3D extent;
	vk::ImageType type = vk::ImageType::e2D;
	uint32_t mipmapLevel = 1;
	uint32_t arrayLayers = 1;
	vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;
	vk::ImageCreateFlags createFlag;
	vk::SharingMode sharingMode = vk::SharingMode::eExclusive;
	vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
	vk::ImageUsageFlags usage;
	uint32_t queueFamilyCount;
	uint32_t* pQueueFamilyIndices;
};

class Image {
public:
	Image() = delete;
	Image(const Device&, vk::Image, ImageInfo);
	Image(const Device&, ImageInfo);

	~Image();

	vk::Image getHandle() const;

	VmaAllocation getAllocation();

	ImageInfo getImageInfo() {
		return imageInfo;
	}

protected:
	const Device& device;

	vk::Image handle;

	VmaAllocation allocation = nullptr;

	ImageInfo imageInfo;
};