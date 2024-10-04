//
//  CommandPool.cpp
//  ToyEngine
//
//  Created by Li Han on 9/25/24.
//

#include "CommandPool.hpp"
#include "Device.hpp"

CommandPool::CommandPool(Device &device, uint32_t queueFamilyIndex, uint32_t commandBufferCount)
:device(device),
queueFamilyIndex(queueFamilyIndex)
{
    vk::CommandPoolCreateInfo createInfo;
    createInfo.queueFamilyIndex = queueFamilyIndex; 
    createInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    
    handle = device.getHandle().createCommandPool(createInfo);

    commandBuffers.resize(commandBufferCount);

    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.commandPool = handle;
    allocateInfo.commandBufferCount = commandBufferCount;
    allocateInfo.level = vk::CommandBufferLevel::ePrimary;

    commandBuffers = device.getHandle().allocateCommandBuffers(allocateInfo);
}

CommandPool::~CommandPool(){
    device.getHandle().destroyCommandPool(handle);
}

vk::CommandBuffer CommandPool::getCommandBuffer(uint32_t index)
{
    return commandBuffers[index];
}
