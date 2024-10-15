#include "Buffer.hpp"

#include "Device.hpp"

Buffer::Buffer(const Device& device, uint64_t size, vk::BufferUsageFlags usage)
	:device{ device }
{
	uint32_t familyIndices[2] = {0, 2};
	vk::BufferCreateInfo createInfo;
	createInfo.usage = usage;
	createInfo.queueFamilyIndexCount = 2;
	createInfo.pQueueFamilyIndices = familyIndices;
	createInfo.sharingMode = vk::SharingMode::eConcurrent;
	createInfo.size = size;

	auto cCreateInfo = static_cast<VkBufferCreateInfo>(createInfo);

	VmaAllocationCreateInfo allocationInfo;
	allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;

	VkBuffer cHandle;
	VmaAllocation allocation;
	vmaCreateBuffer(device.getAllocator(), &cCreateInfo, &allocationInfo, &cHandle, &allocation, nullptr);

	handle = static_cast<vk::Buffer>(cHandle);
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
	void* data;
	vmaMapMemory(device.getAllocator(), allocation, &data);
	memcpy(data, bufferData, bufferSize);
	vmaUnmapMemory(device.getAllocator(), allocation);
}
