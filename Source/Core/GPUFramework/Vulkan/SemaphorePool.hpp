#pragma once

#include "VkCommon.hpp"

#include <atomic>
#include <queue>

class Device;

class SemaphorePool {
public:
	SemaphorePool() = delete;
	SemaphorePool(Device&);

	~SemaphorePool();

	vk::Semaphore requestSemaphore();

	void returnSemaphore(vk::Semaphore);

protected:
	Device& device;

	std::atomic<size_t> availableCount = 0;

	std::queue<vk::Semaphore> semaphores;
};