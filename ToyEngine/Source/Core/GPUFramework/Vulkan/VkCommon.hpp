//
//  vkCommon.h
//  VulkanTest
//
//  Created by Li Han on 9/22/24.
//
#pragma once

#include "Core/Marcos.h"

#if defined ASRCH_OS_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#elif defined ASRCH_OS_MACOS
#define VK_USE_PLATFORM_METAL_EXT
#include <vulkan/vulkan.hpp>
#define VK_ENABLE_PORTABLE
#endif

//#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#define VK_ENABLE_VALIDATION

#include <stdexcept>
#include <string>
#include <vector>

inline const std::string vk_string(VkResult result)
{
    switch (result)
    {
#define STR(r)   \
    case VK_##r: \
        return #r
        STR(NOT_READY);
        STR(TIMEOUT);
        STR(EVENT_SET);
        STR(EVENT_RESET);
        STR(INCOMPLETE);
        STR(ERROR_OUT_OF_HOST_MEMORY);
        STR(ERROR_OUT_OF_DEVICE_MEMORY);
        STR(ERROR_INITIALIZATION_FAILED);
        STR(ERROR_DEVICE_LOST);
        STR(ERROR_MEMORY_MAP_FAILED);
        STR(ERROR_LAYER_NOT_PRESENT);
        STR(ERROR_EXTENSION_NOT_PRESENT);
        STR(ERROR_FEATURE_NOT_PRESENT);
        STR(ERROR_INCOMPATIBLE_DRIVER);
        STR(ERROR_TOO_MANY_OBJECTS);
        STR(ERROR_FORMAT_NOT_SUPPORTED);
        STR(ERROR_FRAGMENTED_POOL);
        STR(ERROR_UNKNOWN);
        STR(ERROR_OUT_OF_POOL_MEMORY);
        STR(ERROR_INVALID_EXTERNAL_HANDLE);
        STR(ERROR_FRAGMENTATION);
        STR(ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS);
        STR(PIPELINE_COMPILE_REQUIRED);
        STR(ERROR_SURFACE_LOST_KHR);
        STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
        STR(SUBOPTIMAL_KHR);
        STR(ERROR_OUT_OF_DATE_KHR);
        STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
        STR(ERROR_VALIDATION_FAILED_EXT);
        STR(ERROR_INVALID_SHADER_NV);
#undef STR
        default:
            return "UNKNOWN_ERROR";
    }
}

class VulkanException : public std::runtime_error
{
  public:
    
    /**
     * @brief Vulkan exception constructor
     */
    VulkanException(VkResult result, const std::string &msg = "Vulkan error") :
    result(result),
    std::runtime_error{msg}
    {
        error_message = std::string(std::runtime_error::what()) + std::string{" : "} + vk_string(result);
    }

    /**
     * @brief Returns the Vulkan error code as string
     * @return String message of exception
     */
    const char *what() const noexcept override{
        return error_message.c_str();
    }

    VkResult result;

  private:
    std::string error_message;
};

/// @brief Helper macro to test the result of Vulkan calls which can return an error.
#define VK_CHECK(x)                                                                    \
    do                                                                                 \
    {                                                                                  \
        VkResult err = static_cast<VkResult>(x);                                                              \
        if (err!= VK_SUCCESS)                                                                       \
        {                                                                              \
            throw std::runtime_error("Detected Vulkan error: " + vk_string(err)); \
        }                                                                              \
    } while (0)
