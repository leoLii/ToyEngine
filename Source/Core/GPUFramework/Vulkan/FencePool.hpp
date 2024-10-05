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

protected:
	Device& device;

	std::atomic<size_t> availableCount = 0;

	std::queue<vk::Fence> fences;
};