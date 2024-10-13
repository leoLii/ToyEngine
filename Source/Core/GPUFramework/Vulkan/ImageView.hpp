#pragma once

#include "VkCommon.hpp"

class Device;
//class Image;

class ImageView {
public:
	ImageView() = delete;
	ImageView(
		const Device&,
		vk::Image,
		vk::ImageViewType,
		vk::Format,
		vk::ComponentMapping,
		vk::ImageSubresourceRange);
	~ImageView();

	vk::ImageView getHandle() const;

protected:
	const Device& device;

	vk::ImageView handle;
	vk::Image image;
	vk::ImageViewType imageViewType;
	vk::Format imageFormat;
	vk::ComponentMapping componentMapping;
	vk::ImageSubresourceRange imageSubresourceRange;
};