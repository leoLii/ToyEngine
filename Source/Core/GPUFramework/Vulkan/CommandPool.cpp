//
//  CommandPool.cpp
//  ToyEngine
//
//  Created by Li Han on 9/25/24.
//

#include "CommandPool.hpp"
#include "Device.hpp"

CommandPool::CommandPool(const Device &device, uint32_t queueFamilyIndex)
:device(device),
queueFamilyIndex(queueFamilyIndex)
{
    vk::CommandPoolCreateInfo createInfo;
    createInfo.queueFamilyIndex = queueFamilyIndex; 
    createInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    
    handle = device.getHandle().createCommandPool(createInfo);
}

CommandPool::~CommandPool(){
    device.getHandle().destroyCommandPool(handle);
}

vk::CommandBuffer CommandPool::requestCommandBuffer(vk::CommandBufferLevel level)
{
    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.commandPool = handle;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.level = level;
    auto commandBuffer = device.getHandle().allocateCommandBuffers(allocateInfo);
    return commandBuffer[0];
}
