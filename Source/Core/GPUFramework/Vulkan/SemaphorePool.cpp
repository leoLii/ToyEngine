#include "SemaphorePool.h"

#include "Device.hpp"

SemaphorePool::SemaphorePool(Device& device)
	:device(device)
{

}

SemaphorePool::~SemaphorePool() {
	while (!semaphores.empty()) {
		auto semaphore = semaphores.front();
		device.getHandle().destroySemaphore(semaphore);
		semaphores.pop();
	}
}

vk::Semaphore SemaphorePool::requestSemaphore() {
	while (true) {
		size_t count = availableCount.load();
		if (count == 0) {
			vk::SemaphoreCreateInfo createInfo;
			auto semaphore = device.getHandle().createSemaphore(createInfo);
			return semaphore;
		}

		if (availableCount.compare_exchange_weak(count, count - 1)) {
			auto semaphore = semaphores.front();
			semaphores.pop();
			return semaphore;
		}
	}
}

void SemaphorePool::returnSemaphore(vk::Semaphore semaphore) {
	semaphores.push(semaphore);
	availableCount.fetch_add(1);
}