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
    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.queueFamilyIndex = queueFamilyIndex;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    
    auto result = vkCreateCommandPool(device.getHandle(), &createInfo, nullptr, &handle);
    
    if(result!=VK_SUCCESS){
        throw VulkanException(result);
    }
}

CommandPool::~CommandPool(){
    vkDestroyCommandPool(device.getHandle(), handle, nullptr);
}
