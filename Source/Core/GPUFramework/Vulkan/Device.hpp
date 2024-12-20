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
#include <tuple>

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
    
    vk::PhysicalDevice getUsingGPU() const;
    
    vk::Device getHandle() const;

    const std::vector<vk::QueueFamilyProperties> getQueueFamilyProperties() const;

    std::tuple<uint32_t, vk::Queue> getQueue(QueueType) const;

    vk::Queue getTextureLoadQueue() const;

    const VmaAllocator& getAllocator() const {
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

    vk::Queue textureLoadQueue{ VK_NULL_HANDLE };

    uint32_t graphicsFamilyIndex = 0;
    uint32_t computeFamilyIndex = 0;
    uint32_t transferFamilyIndex = 1;
    uint32_t presentFamilyIndex = 2;

private:
    std::vector<vk::DeviceQueueCreateInfo> createQueueInfos();
    uint32_t getQueueFamilyIndex(vk::QueueFlagBits);
};
