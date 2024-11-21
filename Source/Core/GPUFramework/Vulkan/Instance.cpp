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
    availableLayers = vk::enumerateInstanceLayerProperties();

    availableExtensions = vk::enumerateInstanceExtensionProperties();
    
    vk::ApplicationInfo applicationInfo;
    applicationInfo.pApplicationName = applicationName.c_str();
    applicationInfo.applicationVersion = 1;
    applicationInfo.pEngineName = "ToyEngine";
    applicationInfo.engineVersion = 1;
    applicationInfo.apiVersion = VK_API_VERSION_1_3;

#ifdef VK_ENABLE_VALIDATION
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo;
    debugUtilsCreateInfo.flags = {};
    debugUtilsCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    debugUtilsCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
    debugUtilsCreateInfo.pfnUserCallback = debug_utils_messenger_callback;
#endif
    
    vk::InstanceCreateInfo createInfo;
#ifdef VK_ENABLE_PORTABLE
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    createInfo.pApplicationInfo = &applicationInfo;
    createInfo.enabledLayerCount = validationLayers.size();
    createInfo.ppEnabledLayerNames = validationLayers.data();
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
#ifdef VK_ENABLE_VALIDATION
    createInfo.pNext = &debugUtilsCreateInfo;
#endif
  
    VK_CHECK(vk::createInstance(&createInfo, nullptr, &handle));

    functionLoader = vk::DispatchLoaderDynamic(handle, vkGetInstanceProcAddr);
#ifdef VK_ENABLE_VALIDATION
    debugMessenger = handle.createDebugUtilsMessengerEXT(debugUtilsCreateInfo, nullptr, functionLoader);
#endif
}

vk::Instance Instance::getHandle() const {
    return this->handle;
}

Instance::~Instance(){
#ifdef VK_ENABLE_VALIDATION
    handle.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, functionLoader);
#endif
    handle.destroy();
}
