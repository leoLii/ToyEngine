//
//  Device.h
//  ToyEngine
//
//  Created by Li Han on 9/24/24.
//

#pragma once

#include "VkCommon.hpp"

#include <vector>
#include <unordered_map>
#include <optional>

class Instance;
class Queue;

class Device {
public:
    Device() = delete;
    Device(const Device&) = delete;
    Device(Device&&) = delete;
    Device &operator=(const Device &) = delete;
    Device &operator=(Device &&) = delete;
    ~Device();

    Device(Instance&);
    
    vk::PhysicalDevice getUsingGPU();
    
    vk::Device getHandle();
    
    vk::Queue getGraphicsQueue(){
        return this->graphicsQueue;
    }
    
    vk::Queue getComputeQueue(){
        return this->computeQueue;
    }
    
    vk::Queue getTransferQueue(){
        return this->transferQueue;
    }
    
    vk::Queue getPresentQueue(){
        return this->presentQueue;
    }

    VmaAllocator getAllocator() {
        return this->allocator;
    }
    
private:
    void initGPU();
    
protected:
    VmaAllocator allocator;

    Instance& instance;
    
    vk::PhysicalDevice gpu{ VK_NULL_HANDLE };
    
    vk::Device handle{ VK_NULL_HANDLE };

    vk::DispatchLoaderDynamic functionLoader;
    
    std::vector<vk::ExtensionProperties> gpuExtensions;
    
    std::vector<const char *> enabledExtensions;
    
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties;
    
    vk::PhysicalDeviceProperties gpuInfo;
    
    vk::PhysicalDeviceMemoryProperties memoryProperty;
    
    vk::PhysicalDeviceFeatures deviceFeatures;
    
    vk::Queue graphicsQueue{ VK_NULL_HANDLE };
    vk::Queue computeQueue{ VK_NULL_HANDLE };
    vk::Queue presentQueue{ VK_NULL_HANDLE };
    vk::Queue transferQueue{ VK_NULL_HANDLE };
};
