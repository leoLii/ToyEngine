//
//  SwapChain.hpp
//  ToyEngine
//
//  Created by Li Han on 9/25/24.
//

#pragma once

#include "VkCommon.hpp"

class Device;

class Swapchain{
public:
    Swapchain() = delete;
    Swapchain(Device&, vk::SurfaceKHR&);

    ~Swapchain();

    const vk::SwapchainKHR getHandle() const;

    std::vector<vk::Image> getSwapChainImages() const;

    vk::Format getFormat();

    vk::Extent2D getExtent();

    void rebuildWithSize(vk::Extent2D);

    void rebuild();
    
protected:
    Device& device;

    vk::SurfaceKHR surface;
    vk::SwapchainKHR handle;

    vk::SurfaceCapabilitiesKHR surfaceCapabilities;
    std::vector<vk::SurfaceFormatKHR> surfaceFormats;
    std::vector<vk::PresentModeKHR> presentModes;

    std::vector<vk::Image> images;

    vk::Format format;
    vk::Extent2D extent;

    vk::ImageCompressionFlagsEXT requestedCompression{ vk::ImageCompressionFlagBitsEXT::eDefault };

    std::vector<vk::ImageCompressionFixedRateFlagsEXT> requestedCompressionFixedRate{ vk::ImageCompressionFixedRateFlagBitsEXT::eNone };
};
