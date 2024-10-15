#include "BufferView.hpp"
#include "Device.hpp"

BufferView::BufferView(
	const Device& device, 
	vk::Buffer buffer, 
	vk::Format format, 
	vk::DeviceSize offset, 
	vk::DeviceSize range)
	:device{device},
	buffer{buffer},
	format{format},
	offset{offset},
	range{range}
{
	vk::BufferViewCreateInfo createInfo;
	createInfo.buffer = buffer;
	createInfo.format = format;
	createInfo.offset = offset;
	createInfo.range = range;

	handle = device.getHandle().createBufferView(createInfo);
}

BufferView::~BufferView()
{
	device.getHandle().destroyBufferView(handle);
}

vk::BufferView BufferView::getHandle() const {
	return handle;
}
