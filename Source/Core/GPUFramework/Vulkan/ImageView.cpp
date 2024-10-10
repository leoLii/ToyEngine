#include "ImageView.hpp"
#include "Device.hpp"

ImageView::ImageView(const Device& device, const vk::Image image, const vk::ImageViewType type, const vk::Format format, const vk::ComponentMapping components, const vk::ImageSubresourceRange range):
	device(device),
	image(image),
	imageViewType(type),
	imageFormat(format),
	componentMapping(components),
	imageSubresourceRange(range)
{
	vk::ImageViewCreateInfo createInfo;
	createInfo.image = image;
	createInfo.viewType = type;
	createInfo.format = format;
	createInfo.components = components;
	createInfo.subresourceRange = range;
	handle = device.getHandle().createImageView(createInfo);
}

ImageView::~ImageView() {
	device.getHandle().destroyImageView(handle);
}

vk::ImageView ImageView::getHandle() const
{
	return handle;
}
