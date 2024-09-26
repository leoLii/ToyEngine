//
//  instance.cpp
//  VulkanTest
//
//  Created by Li Han on 9/24/24.
//

#include "Instance.hpp"
//#include "../../Common/logging.hpp"
#include <iostream>


//#include <Volk/volk.h>

#ifdef VK_ENABLE_VALIDATION

VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                              const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                                                              void                                       *user_data)
{
    std::cerr << "validation layer: " << callback_data->pMessage << std::endl;
//    // Log debug message
//    if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
//    {
//        LOGW("{} - {}: {}", callback_data->messageIdNumber, callback_data->pMessageIdName, callback_data->pMessage);
//    }
//    else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
//    {
//        LOGE("{} - {}: {}", callback_data->messageIdNumber, callback_data->pMessageIdName, callback_data->pMessage);
//    }
//    return VK_FALSE;
    return VK_FALSE;
}

#endif

Instance::Instance(const std::string                            &applicationName,
                   const std::vector<const char *>              &extensions,
                   const std::vector<const char *>              &validationLayers,
                   bool                                          headless,
                   uint32_t                                      api_version)
{
    uint32_t layerCount;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
    availableLayers.resize(layerCount);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()));
    
    uint32_t extensionCount;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
    availableExtensions.resize(extensionCount);
    VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data()));
    
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = applicationName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "ToyEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = api_version;
    
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<int>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
#ifdef VK_ENABLE_PORTABLE
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    createInfo.enabledLayerCount = static_cast<int>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  
    auto result = VK_SUCCESS;
    
#ifdef VK_ENABLE_VALIDATION
    VkDebugUtilsMessengerCreateInfoEXT debug_utils_create_info  = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debug_utils_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    debug_utils_create_info.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_utils_create_info.pfnUserCallback = debug_utils_messenger_callback;
    
    createInfo.pNext = &debug_utils_create_info;
#endif
    
    result = vkCreateInstance(&createInfo, nullptr, &handle);
    if(result != VK_SUCCESS){
        throw VulkanException(result, "Failed to create instance: ");
    }
    
#ifdef VK_ENABLE_VALIDATION
    vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(handle, "vkCreateDebugUtilsMessengerEXT");
    if (!vkCreateDebugUtilsMessengerEXT) {
        std::runtime_error("Failed load function");
    }
    
    vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(handle, "vkDestroyDebugUtilsMessengerEXT");
    if(vkDestroyDebugUtilsMessengerEXT==nullptr){
        std::runtime_error("Failed load function");
    }
    
    result = vkCreateDebugUtilsMessengerEXT(handle, &debug_utils_create_info, nullptr, &debugMessenger);
    if (result != VK_SUCCESS)
    {
        throw VulkanException(result, "Could not create debug utils messenger");
    }
#endif
}

VkInstance Instance::getHandle(){
    return this->handle;
}

Instance::~Instance(){
    vkDestroyDebugUtilsMessengerEXT(handle, debugMessenger, nullptr);
    vkDestroyInstance(handle, nullptr);
}
