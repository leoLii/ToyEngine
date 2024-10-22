#include "ImageView.hpp"
#include "Device.hpp"
#include "Image.hpp"

ImageView::ImageView(const Device& device, Image* image, vk::ImageViewType type, vk::Format format, vk::ComponentMapping components, vk::ImageSubresourceRange range):
	device(device),
	image(image),
	imageViewType(type),
	imageFormat(format),
	componentMapping(components),
	imageSubresourceRange(range)
{
	vk::ImageViewCreateInfo createInfo;
	createInfo.image = image->getHandle();
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
