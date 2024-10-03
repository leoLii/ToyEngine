//
//  CommandPool.cpp
//  ToyEngine
//
//  Created by Li Han on 9/25/24.
//

#include "CommandPool.hpp"
#include "Device.hpp"

CommandPool::CommandPool(Device &device, uint32_t queueFamilyIndex, size_t thread_index)
:device(device),
queueFamilyIndex(queueFamilyIndex),
threadIndex(thread_index)
{
    vk::CommandPoolCreateInfo createInfo;
    createInfo.queueFamilyIndex = queueFamilyIndex; 
    createInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    
    handle = device.getHandle().createCommandPool(createInfo);
}

CommandPool::~CommandPool(){
    device.getHandle().destroyCommandPool(handle);
}
