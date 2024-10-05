#pragma once

#include "VkCommon.hpp"

#include <atomic>
#include <queue>

class Device;

class FencePool {
public:
	FencePool() = delete;
	FencePool(Device&);

	~FencePool();

	vk::Fence requestFence();

	void returnFence(vk::Fence);

	void resetFences(vk::Fence);

	void waitForFences(vk::Fence, VkBool32 waitForAll = VK_TRUE, uint32_t timeout = std::numeric_limits<uint32_t>::max());

	void resetFences(std::vector<vk::Fence>);

	void waitForFences(std::vector<vk::Fence>, VkBool32 waitForAll = VK_TRUE, uint32_t timeout = std::numeric_limits<uint32_t>::max());

protected:
	Device& device;

	std::atomic<size_t> availableCount = 0;

	std::queue<vk::Fence> fences;
};