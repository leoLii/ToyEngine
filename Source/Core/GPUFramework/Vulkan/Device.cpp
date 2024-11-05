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

uint32_t Device::getQueueFamilyIndex(vk::QueueFlagBits queueFlag)
{
    // Dedicated queue for compute
    // Try to find a queue family index that supports compute but not graphics
    if (queueFlag & vk::QueueFlagBits::eCompute)
    {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
        {
            if ((queueFamilyProperties[i].queueFlags & queueFlag) && 
                !(queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics))
            {
                return i;
                break;
            }
        }
    }

    // Dedicated queue for transfer
    // Try to find a queue family index that supports transfer but not graphics and compute
    if (queueFlag & vk::QueueFlagBits::eTransfer)
    {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
        {
            if ((queueFamilyProperties[i].queueFlags & queueFlag) && 
                !(queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) && 
                !(queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eCompute))
            {
                return i;
                break;
            }
        }
    }

    // For other queue types or if no separate compute queue is present, return the first one to support the requested flags
    for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
    {
        if (queueFamilyProperties[i].queueFlags & queueFlag)
        {
            return i;
            break;
        }
    }

    throw std::runtime_error("Could not find a matching queue family index");
}

std::vector<vk::DeviceQueueCreateInfo> Device::createQueueInfos()
{
    uint32_t  queueFamilyCount = static_cast<uint32_t>(queueFamilyProperties.size());
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos(queueFamilyCount, vk::DeviceQueueCreateInfo{});
    std::vector<std::vector<float>> queue_priorities(queueFamilyCount);

    for (uint32_t queueFamilyIndex = 0U; queueFamilyIndex < queueFamilyCount; ++queueFamilyIndex)
    {
        const vk::QueueFamilyProperties& queueFamilyProperty = queueFamilyProperties[queueFamilyIndex];
            uint32_t graphicsQueueFamily = getQueueFamilyIndex(vk::QueueFlagBits::eGraphics);
            if (graphicsQueueFamily == queueFamilyIndex)
            {
                queue_priorities[queueFamilyIndex].reserve(queueFamilyProperty.queueCount);
                queue_priorities[queueFamilyIndex].push_back(0.9f);
                for (uint32_t i = 1; i < queueFamilyProperty.queueCount; i++)
                {
                    queue_priorities[queueFamilyIndex].push_back(0.5f);
                }
            }
            else
            {
                queue_priorities[queueFamilyIndex].resize(queueFamilyProperty.queueCount, 0.5f);
            }

        vk::DeviceQueueCreateInfo& queueCreateInfo = queueCreateInfos[queueFamilyIndex];

        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.queueCount = queueFamilyProperty.queueCount;
        queueCreateInfo.pQueuePriorities = queue_priorities[queueFamilyIndex].data();
    }
    return queueCreateInfos;
}

Device::Device(Instance& instance)
    :instance(instance)
{
    initGPU();

    vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature;
    dynamicRenderingFeature.dynamicRendering = true;
    
    auto queueInfos = createQueueInfos();
    auto queueFamilyCount = queueFamilyProperties.size();
    vk::DeviceCreateInfo createInfo{};
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueFamilyCount);
    createInfo.pQueueCreateInfos = queueInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    std::vector<const char*> enabledExtensions = { 
        VK_KHR_SWAPCHAIN_EXTENSION_NAME, 
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
        VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
        VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME};
#ifdef ARCH_OS_MACOS
    enabledExtensions.push_back("VK_KHR_portability_subset");
#endif // ARCH_OS_MACOS

    createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();
    createInfo.pNext = &dynamicRenderingFeature;
    
    handle = gpu.createDevice(createInfo);


    graphicsQueue = handle.getQueue(0, 0);
    presentQueue = handle.getQueue(0, 1);
    computeQueue = handle.getQueue(0, 2);
    transferQueue = handle.getQueue(0, 3);

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
