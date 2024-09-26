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

void Device::initGPU(){
    uint32_t gpuCount = 0;
    vkEnumeratePhysicalDevices(instance.getHandle(), &gpuCount, nullptr);
    if (gpuCount == 0) { throw std::runtime_error("failed to find GPUs with Vulkan support!"); }
    std::vector<VkPhysicalDevice> gpus(gpuCount);
    vkEnumeratePhysicalDevices(instance.getHandle(), &gpuCount, gpus.data());
    
    gpu = gpus[0];
    if (gpu == VK_NULL_HANDLE) { throw std::runtime_error("failed to find a suitable GPU!"); }
    
    vkGetPhysicalDeviceFeatures2(gpu, &deviceFeatures);
    vkGetPhysicalDeviceProperties2(gpu, &gpuInfo);
    vkGetPhysicalDeviceMemoryProperties2(gpu, &memoryProperty);
    
    std::cout<<"Found: "<<gpuInfo.properties.deviceName<<std::endl;
}

Device::Device(Instance& instance)
    :instance(instance)
{
    initGPU();
    
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, nullptr);
    gpuExtensions.resize(extensionCount);
    vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, gpuExtensions.data());
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2(gpu, &queueFamilyCount, nullptr);
    queueFamilyProperties.resize(queueFamilyCount);
    std::fill(queueFamilyProperties.begin(), queueFamilyProperties.end(), VkQueueFamilyProperties2{VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2});
    vkGetPhysicalDeviceQueueFamilyProperties2(gpu, &queueFamilyCount, queueFamilyProperties.data());
    
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;
    for(int index = 0; index < queueFamilyCount; index++){
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = index;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueFamilyProperties.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures.features;
    std::vector<const char *> enabledExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#ifdef ARCH_OS_MACOS
    enabledExtensions.push_back("VK_KHR_portability_subset");
#endif // ARCH_OS_MACOS

    createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();
    
    auto result = vkCreateDevice(gpu, &createInfo, nullptr, &handle);
    if(result!=VK_SUCCESS){
        throw VulkanException(result);
    }
    
    vkGetDeviceQueue(handle, 0, 0, &graphicsQueue);
    vkGetDeviceQueue(handle, 2, 0, &presentQueue);
    vkGetDeviceQueue(handle, 1, 0, &computeQueue);
    vkGetDeviceQueue(handle, 1, 1, &transferQueue);
}

VkPhysicalDevice Device::getUsingGPU(){
    return this->gpu;
}

VkDevice Device::getHandle(){
    return this->handle;
}
