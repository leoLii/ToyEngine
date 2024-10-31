#include "ImageView.hpp"
#include "Device.hpp"
#include "Image.hpp"

ImageView::ImageView(const Device& device, Image* image, ImageViewInfo viewInfo):
	device{ device },
	image{ image },
	viewInfo{ viewInfo }
{
	vk::ImageViewCreateInfo createInfo;
	createInfo.image = image->getHandle();
	createInfo.viewType = viewInfo.type;
	createInfo.format = viewInfo.format;
	createInfo.components = viewInfo.mapping;
	createInfo.subresourceRange = viewInfo.range;
	handle = device.getHandle().createImageView(createInfo);
}

ImageView::~ImageView() {
	device.getHandle().destroyImageView(handle);
}

vk::ImageView ImageView::getHandle() const
{
	return handle;
}
