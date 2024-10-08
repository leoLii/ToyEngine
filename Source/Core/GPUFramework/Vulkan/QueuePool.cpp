#include "QueuePool.hpp"
#include "Device.hpp"

#include "Platform/Window.hpp"

QueuePool::QueuePool(const Device& device, const Window& window)
	:device{ device }
	, window{ window }
{
	/*auto queueFamilyProperties = device.getQueueFamilyProperties();
	for (int i = 0; i < queueFamilyProperties.size(); i++) {
		auto property = queueFamilyProperties[i];
		auto flag = static_cast<QueueFunctionFlags>(property.queueFlags | vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer);
		if (device.getUsingGPU().getSurfaceSupportKHR(i, window.getSurface()))
			flag |= (1 << 3);
		if (flag) {
			for (int j = 0; j < property.queueCount; j++) {
				queues.push(Queue(device.getHandle().getQueue(i, j, flag)));
				availableCount.fetch_add(1);
			}
		}
	}*/
}

//const Queue& QueuePool::acquireQueue(const QueueFunctionFlags) const
//{
//}