#pragma once

#include "VkCommon.hpp"

class Device;

class Buffer {
public:
	NO_MOVE(Buffer);
	NO_COPY(Buffer);

	Buffer() = delete;
	Buffer(const Device&, uint64_t, vk::BufferUsageFlags);
	~Buffer();

	vk::Buffer getHandle() const;

	VmaAllocation getAllocation();

	void copyToGPU(const void*, const uint64_t);

protected:
	const Device& device;

	vk::Buffer handle;

	VmaAllocation allocation = nullptr;
	VmaAllocationInfo allocInfo;
};