//
//  instance.h
//  VulkanTest
//
//  Created by Li Han on 9/23/24.
//

#pragma once

#include "VkCommon.hpp"

#include <optional>
#include <unordered_map>
#include <vector>
#include <string>

class PhysicalDevice;

class Instance {
public:
    Instance() = delete;
    Instance(const Instance&) = delete;
    Instance(Instance&&) = delete;
    Instance &operator=(const Instance &) = delete;
    Instance &operator=(Instance &&) = delete;
    ~Instance();
    
    Instance(const std::string                            &application_name,
             const std::vector<const char *>              &extensions        = {},
             const std::vector<const char *>              &validationLayers = {},
             bool                                          headless                   = false,
             uint32_t                                      api_version                = VK_API_VERSION_1_3);
    
    
    vk::Instance getHandle() const;
    
protected:
    vk::Instance handle{VK_NULL_HANDLE};

    vk::DispatchLoaderDynamic functionLoader;
    
    std::vector<vk::ExtensionProperties> availableExtensions;
    
    std::vector<vk::LayerProperties> availableLayers;
    
#ifdef VK_ENABLE_VALIDATION
    vk::DebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};
#endif
};
