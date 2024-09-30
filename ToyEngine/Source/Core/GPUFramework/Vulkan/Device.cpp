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

void Device::initGPU() {
    gpu = instance.getHandle().enumeratePhysicalDevices().front();
    if (!gpu) { throw std::runtime_error("failed to find a suitable GPU!"); }
    
    deviceFeatures = gpu.getFeatures();
    gpuInfo = gpu.getProperties();
    memoryProperty = gpu.getMemoryProperties();
    
    std::cout<<"Found: "<<gpuInfo.deviceName<<std::endl;
}

Device::Device(Instance& instance)
    :instance(instance)
{
    initGPU();

    gpuExtensions = gpu.enumerateDeviceExtensionProperties();

    queueFamilyProperties = gpu.getQueueFamilyProperties();
    
    
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;
    for(int index = 0; index < queueFamilyProperties.size(); index++){
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.queueFamilyIndex = index;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    vk::DeviceCreateInfo createInfo{};
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueFamilyProperties.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    std::vector<const char*> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
#ifdef ARCH_OS_MACOS
    enabledExtensions.push_back("VK_KHR_portability_subset");
#endif // ARCH_OS_MACOS

    createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();
    
    handle = gpu.createDevice(createInfo);

    graphicsQueue = handle.getQueue(0, 0);
    presentQueue = handle.getQueue(2, 0);
    computeQueue = handle.getQueue(1, 0);
    transferQueue = handle.getQueue(1, 1);
}


Device::~Device() {
    handle.destroy();
}

vk::PhysicalDevice Device::getUsingGPU(){
    return this->gpu;
}

vk::Device Device::getHandle(){
    return this->handle;
}
