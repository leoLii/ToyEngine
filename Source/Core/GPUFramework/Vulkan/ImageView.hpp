#pragma once

#include "VkCommon.hpp"

class Device;
//class Image;

class ImageView {
public:
	ImageView() = delete;
	ImageView(
		const Device&,
		const vk::Image,
		const vk::ImageViewType,
		const vk::Format,
		const vk::ComponentMapping,
		const vk::ImageSubresourceRange);
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