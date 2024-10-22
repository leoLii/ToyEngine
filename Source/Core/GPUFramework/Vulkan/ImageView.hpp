#pragma once

#include "VkCommon.hpp"
#include "Image.hpp"

class Device;

class ImageView {
public:
	ImageView() = delete;
	ImageView(
		const Device&,
		Image*,
		vk::ImageViewType,
		vk::Format,
		vk::ComponentMapping,
		vk::ImageSubresourceRange);
	~ImageView();

	vk::ImageView getHandle() const;

protected:
	const Device& device;

	vk::ImageView handle;
	Image* image;
	vk::ImageViewType imageViewType;
	vk::Format imageFormat;
	vk::ComponentMapping componentMapping;
	vk::ImageSubresourceRange imageSubresourceRange;
};