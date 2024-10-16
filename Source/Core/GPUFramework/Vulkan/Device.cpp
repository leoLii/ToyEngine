//
//  Device.cpp
//  ToyEngine
//
//  Created by Li Han on 9/24/24.
//

#include "Device.hpp"
#include "Instance.hpp"
//#include "Queue.hpp"
#include <iostream>

void Device::initGPU()
{
    gpu = instance.getHandle().enumeratePhysicalDevices().front();
    if (!gpu) { throw std::runtime_error("failed to find a suitable GPU!"); }
    
    deviceFeatures = gpu.getFeatures();
    gpuInfo = gpu.getProperties();
    memoryProperty = gpu.getMemoryProperties();
    gpuExtensions = gpu.enumerateDeviceExtensionProperties();
    queueFamilyProperties = gpu.getQueueFamilyProperties();
    
    std::cout<<"Found: "<<gpuInfo.deviceName<<std::endl;
}

Device::Device(Instance& instance)
    :instance(instance)
{
    initGPU();

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;
    for(int index = 0; index < queueFamilyProperties.size(); index++){
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.queueFamilyIndex = index;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature;
    dynamicRenderingFeature.dynamicRendering = true;
    
    vk::DeviceCreateInfo createInfo{};
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueFamilyProperties.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    std::vector<const char*> enabledExtensions = { 
        VK_KHR_SWAPCHAIN_EXTENSION_NAME, 
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
        VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
        VK_KHR_MAINTENANCE_5_EXTENSION_NAME};
#ifdef ARCH_OS_MACOS
    enabledExtensions.push_back("VK_KHR_portability_subset");
#endif // ARCH_OS_MACOS

    createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();
    createInfo.pNext = &dynamicRenderingFeature;
    
    handle = gpu.createDevice(createInfo);

    graphicsQueue = handle.getQueue(0, 0);
    presentQueue = handle.getQueue(2, 0);
    //computeQueue = handle.getQueue(1, 0);
    //transferQueue = handle.getQueue(1, 1);

    // init allocator
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    allocatorCreateInfo.physicalDevice = gpu;
    allocatorCreateInfo.device = handle;
    allocatorCreateInfo.instance = instance.getHandle();
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    vmaCreateAllocator(&allocatorCreateInfo, &allocator);
}


Device::~Device() {
    vmaDestroyAllocator(allocator);
    handle.destroy();
}

vk::PhysicalDevice Device::getUsingGPU() const 
{
    return this->gpu;
}

vk::Device Device::getHandle() const
{
    return this->handle;
}

const std::vector<vk::QueueFamilyProperties> Device::getQueueFamilyProperties() const
{
    return queueFamilyProperties;
}
