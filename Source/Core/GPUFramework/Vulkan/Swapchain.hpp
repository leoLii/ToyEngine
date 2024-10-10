//
//  SwapChain.hpp
//  ToyEngine
//
//  Created by Li Han on 9/25/24.
//

#pragma once

#include "VkCommon.hpp"
#include "Image.hpp"
#include "ImageView.hpp"

class Device;

class Swapchain{
public:
    Swapchain() = delete;
    Swapchain(const Device&, const vk::SurfaceKHR);

    ~Swapchain();

    vk::SwapchainKHR getHandle() const;

    const std::vector<Image>& getImages() const;

    const std::vector<ImageView>& getImageViews() const;

    ImageInfo getSwapchainImageInfo() const;

    uint32_t getImageCount() const;

    void rebuildWithSize(vk::Extent2D);

    void rebuild();
    
protected:
    const Device& device;

    vk::SurfaceKHR surface;
    vk::SwapchainKHR handle;

    ImageInfo imageInfo;

    vk::SurfaceCapabilitiesKHR surfaceCapabilities;
    std::vector<vk::SurfaceFormatKHR> surfaceFormats;
    std::vector<vk::PresentModeKHR> presentModes;

    uint32_t imageCount = 3;
    std::vector<Image> images;
    std::vector<ImageView> imageViews;

    vk::ImageCompressionFlagsEXT requestedCompression{ vk::ImageCompressionFlagBitsEXT::eDefault };

    std::vector<vk::ImageCompressionFixedRateFlagsEXT> requestedCompressionFixedRate{ vk::ImageCompressionFixedRateFlagBitsEXT::eNone };
};
