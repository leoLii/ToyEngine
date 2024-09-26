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
    
    
    VkInstance getHandle();
    
#ifdef VK_ENABLE_VALIDATION
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = {VK_NULL_HANDLE};
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = {VK_NULL_HANDLE};
#endif
    
protected:
    VkInstance handle{VK_NULL_HANDLE};
    
    std::vector<VkExtensionProperties> availableExtensions;
    
    std::vector<VkLayerProperties> availableLayers;
    
#ifdef VK_ENABLE_VALIDATION
    VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};
#endif
};
