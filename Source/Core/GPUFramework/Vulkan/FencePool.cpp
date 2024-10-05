#include "FencePool.hpp"

#include "Device.hpp"

FencePool::FencePool(Device& device)
	:device(device) {

}

FencePool::~FencePool() {
	while (!fences.empty()) {
		auto fence = fences.front();
		device.getHandle().destroyFence(fence);
		fences.pop();
	}
}

vk::Fence FencePool::requestFence() {
	while (true) {
		size_t count = availableCount.load();
		if (count == 0) {
			vk::FenceCreateInfo createInfo;
			createInfo.flags = vk::FenceCreateFlagBits::eSignaled;
			auto fence = device.getHandle().createFence(createInfo);
			return fence;
		}

		if (availableCount.compare_exchange_weak(count, count - 1)) {
			auto fence = fences.front();
			fences.pop();
			return fence;
		}
	}
}

void FencePool::returnFence(vk::Fence fence) {
	fences.push(fence);
	availableCount.fetch_add(1);
}