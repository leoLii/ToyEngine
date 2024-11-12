#pragma once

#include "VkCommon.hpp"

class Device;

class Buffer {
public:
	NO_MOVE(Buffer);
	NO_COPY(Buffer);

	Buffer() = delete;
	Buffer(const Device&, uint64_t, vk::BufferUsageFlags, VmaMemoryUsage=VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO, bool=false);
	~Buffer();

	vk::Buffer getHandle() const;

	VmaAllocation getAllocation();

	void copyToGPU(const void*, const uint64_t);

	const size_t getSize() const;

	const size_t getOffset() const;

protected:
	const Device& device;

	vk::Buffer handle;

	VmaAllocation allocation = nullptr;
	VmaAllocationInfo allocInfo;
};