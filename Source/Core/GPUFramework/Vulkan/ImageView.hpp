#pragma once

#include "VkCommon.hpp"
#include "Image.hpp"

class Device;

struct ImageViewInfo {
	vk::ImageViewType type = vk::ImageViewType::e2D;
	vk::Format format = vk::Format::eR8G8B8A8Unorm;
	vk::ComponentMapping mapping = vk::ComponentMapping{};
	vk::ImageSubresourceRange range = vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
};

class ImageView {
public:
	ImageView() = delete;
	ImageView(const Device&, Image*, ImageViewInfo);
	~ImageView();

	vk::ImageView getHandle() const;

protected:
	const Device& device;

	vk::ImageView handle;
	Image* image;
	ImageViewInfo viewInfo;
};