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
    
    VkPhysicalDevice getUsingGPU();
    
    VkDevice getHandle();
    
    VkQueue getGraphicsQueue(){
        return this->graphicsQueue;
    }
    
    VkQueue getComputeQueue(){
        return this->computeQueue;
    }
    
    VkQueue getTransferQueue(){
        return this->transferQueue;
    }
    
    VkQueue getPresentQueue(){
        return this->presentQueue;
    }
    
private:
    void initGPU();
    
    void setupExtensions();
    
protected:
    Instance& instance;
    
    VkPhysicalDevice gpu{VK_NULL_HANDLE};
    
    VkDevice handle{VK_NULL_HANDLE};
    
    std::vector<VkExtensionProperties> gpuExtensions;
    
    std::vector<const char *> enabledExtensions;
    
    std::vector<VkQueueFamilyProperties2> queueFamilyProperties;
    
    VkPhysicalDeviceProperties2 gpuInfo{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    
    VkPhysicalDeviceMemoryProperties2 memoryProperty{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
    
    VkPhysicalDeviceFeatures2 deviceFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    
    VkQueue graphicsQueue{VK_NULL_HANDLE};
    VkQueue computeQueue{VK_NULL_HANDLE};
    VkQueue presentQueue{VK_NULL_HANDLE};
    VkQueue transferQueue{VK_NULL_HANDLE};
};
