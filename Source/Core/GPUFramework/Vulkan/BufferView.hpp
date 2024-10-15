#pragma once

#include "VkCommon.hpp"

#include "Buffer.hpp"

class Device;

class BufferView {
public:
	NO_MOVE(BufferView);
	NO_COPY(BufferView);

	BufferView() = delete;

	BufferView(
		const Device&,
		vk::Buffer,
		vk::Format,
		vk::DeviceSize,
		vk::DeviceSize);
	~BufferView();

	vk::BufferView getHandle() const;

protected:
	const Device& device;

	vk::BufferView handle;

	vk::Buffer buffer;
	
	vk::Format format;

	vk::DeviceSize offset;

	vk::DeviceSize range;
};