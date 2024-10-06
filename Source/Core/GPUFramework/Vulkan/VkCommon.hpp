//
//  vkCommon.h
//  VulkanTest
//
//  Created by Li Han on 9/22/24.
//
#pragma once

#include "Common/Marcos.h"

#if defined ASRCH_OS_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#elif defined ASRCH_OS_MACOS
#define VK_USE_PLATFORM_METAL_EXT
#include <vulkan/vulkan.hpp>
#define VK_ENABLE_PORTABLE
#endif

#include <vma/vk_mem_alloc.h>

#define VK_ENABLE_VALIDATION

#include <stdexcept>
#include <string>
#include <vector>

inline const std::string vk_string(vk::Result result)
{
	switch (result)
	{
#define STR(r)   \
    case vk::Result::e##r: \
        return #r
		STR(NotReady);
		STR(Timeout);
		STR(EventSet);
		STR(EventReset);
		STR(Incomplete);
		STR(ErrorOutOfHostMemory);
		STR(ErrorOutOfDeviceMemory);
		STR(ErrorInitializationFailed);
		STR(ErrorDeviceLost);
		STR(ErrorMemoryMapFailed);
		STR(ErrorLayerNotPresent);
		STR(ErrorExtensionNotPresent);
		STR(ErrorFeatureNotPresent);
		STR(ErrorIncompatibleDriver);
		STR(ErrorTooManyObjects);
		STR(ErrorFormatNotSupported);
		STR(ErrorFragmentedPool);
		STR(ErrorUnknown);
		STR(ErrorOutOfPoolMemory);
		STR(ErrorInvalidExternalHandle);
		STR(ErrorFragmentation);
		STR(ErrorInvalidOpaqueCaptureAddress);
		STR(PipelineCompileRequired);
		STR(ErrorSurfaceLostKHR);
		STR(ErrorNativeWindowInUseKHR);
		STR(SuboptimalKHR);
		STR(ErrorOutOfDateKHR);
		STR(ErrorIncompatibleDisplayKHR);
		STR(ErrorValidationFailedEXT);
		STR(ErrorInvalidShaderNV);
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
	VulkanException(vk::Result result, const std::string& msg = "Vulkan error") :
		result(result),
		std::runtime_error{ msg }
	{
		error_message = std::string(std::runtime_error::what()) + std::string{ " : " } + vk_string(result);
	}

	/**
	 * @brief Returns the Vulkan error code as string
	 * @return String message of exception
	 */
	const char* what() const noexcept override {
		return error_message.c_str();
	}

	vk::Result result;

private:
	std::string error_message;
};

/// @brief Helper macro to test the result of Vulkan calls which can return an error.
#define VK_CHECK(x)                                                                    \
    do                                                                                 \
    {                                                                                  \
        vk::Result err = x;                                                              \
        if (err!= vk::Result::eSuccess)                                                                       \
        {                                                                              \
            throw std::runtime_error("Detected Vulkan error: " + vk_string(err)); \
        }                                                                              \
    } while (0)
