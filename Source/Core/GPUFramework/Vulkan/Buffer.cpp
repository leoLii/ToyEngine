#include "Buffer.hpp"

#include "Device.hpp"

Buffer::Buffer(const Device& device, uint64_t size, vk::BufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, bool mapped)
	:device{ device }
{
	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = size;
	bufferInfo.usage = static_cast<VkBufferUsageFlags>(bufferUsage);
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo createInfo = {};
	createInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
	if(mapped)
		createInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
	createInfo.usage = memoryUsage;

	VkBuffer buffer;
	auto result = vmaCreateBuffer(device.getAllocator(), &bufferInfo, &createInfo, &buffer, &allocation, &allocInfo);

	handle = static_cast<vk::Buffer>(buffer);
}

Buffer::~Buffer() 
{
	if (allocation) {
		vmaDestroyBuffer(device.getAllocator(), static_cast<VkBuffer>(handle), allocation);
	}
}

vk::Buffer Buffer::getHandle() const
{
	return handle;
}

VmaAllocation Buffer::getAllocation()
{
	return this->allocation;
}

void Buffer::copyToGPU(const void* bufferData, const uint64_t bufferSize)
{
	memcpy(allocInfo.pMappedData, bufferData, bufferSize);
}

const size_t Buffer::getSize() const
{
	return static_cast<size_t>(allocInfo.size);
}

const size_t Buffer::getOffset() const
{
	//return static_cast<size_t>(allocInfo.offset);
	return 0;
}
