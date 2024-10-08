#pragma once

#include "Core/GPUFramework/GPUContext.hpp"

#include "Core/GPUFramework/Vulkan/Instance.hpp"
#include "Core/GPUFramework/Vulkan/Device.hpp"
#include "Core/GPUFramework/Vulkan/CommandPool.hpp"
#include "Core/GPUFramework/Vulkan/ImageView.hpp"
#include "Core/GPUFramework/Vulkan/Swapchain.hpp"
#include "Core/GPUFramework/Vulkan/RenderPass.hpp"
#include "Core/GPUFramework/Vulkan/GraphicsPipeline.hpp"
#include "Core/GPUFramework/Vulkan/SemaphorePool.hpp"
#include "Core/GPUFramework/Vulkan/FencePool.hpp"
#include "Core/GPUFramework/Vulkan/ShaderModule.hpp"
#include "Core/GPUFramework/Vulkan/Framebuffer.hpp"

#include "Platform/Window.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <set>

#include <exception>
#include <functional>
#include <algorithm>
#include <memory>

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_portability_subset" };

int WIDTH = 1920;
int HEIGHT = 1080;

const char* VertShader = R"(
#version 450
precision mediump float;

layout(location = 0) out vec3 out_color;

vec2 triangle_positions[3] = vec2[](
    vec2(0.5, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 triangle_colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main()
{
    gl_Position = vec4(triangle_positions[gl_VertexIndex], 0.5, 1.0);

    out_color = triangle_colors[gl_VertexIndex];
}
)";

const char* FragShader = R"(
#version 450
precision mediump float;

layout(location = 0) in vec3 in_color;

layout(location = 0) out vec4 out_color;

void main()
{
	out_color = vec4(in_color, 1.0);
}
)";

std::shared_ptr<Window> window;

std::unique_ptr<GPUContext> gpuContext;

CommandPool* commandPool;
RenderPass* renderPass;
GraphicsPipeline* graphicsPipeline;
std::vector<Framebuffer*> framebuffers;

VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;

std::vector<VkImage> swapChainImages;
std::vector<VkImageView> swapChainImageViews;
uint32_t currentFrame = 0;
bool framebufferResized = false;

void cleanupSwapChain() {
    for (auto framebuffer : framebuffers) {
        delete framebuffer;
    }
    framebuffers.resize(0);
    for (auto imageView : swapChainImageViews)
    {
        vkDestroyImageView(gpuContext->getDevice()->getHandle(), imageView, nullptr);
    }
}

void createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(gpuContext->getDevice()->getHandle(), &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

void recreateSwapChain()
{
    int width = 0, height = 0;
    window->getFramebufferSize(&width, &height);
    while (width == 0 || height == 0)
    {
        window->getFramebufferSize(&width, &height);
        window->waitEvents();
    }

    vkDeviceWaitIdle(gpuContext->getDevice()->getHandle());

    cleanupSwapChain();
    gpuContext->rebuildSwapchainWithSize(vk::Extent2D(width, height));
    auto spImages = gpuContext->getSwapchainImages();
    swapChainImages.resize(spImages.size());

    std::transform(spImages.begin(), spImages.end(), swapChainImages.begin(), [](vk::Image i)->VkImage {
        return static_cast<VkImage>(i);
        });
    swapChainImageFormat = static_cast<VkFormat>(gpuContext->getSwapchainFormat());
    swapChainExtent = static_cast<VkExtent2D>(gpuContext->getSwapchainExtent());
    //swapchain->rebuildWithSize(vk::Extent2D(width, height));
    createImageViews();

    for (int i = 0; i < swapChainImageViews.size(); i++) {
        std::vector<vk::ImageView> temp = { swapChainImageViews[i] };
        auto f = new Framebuffer{ *gpuContext->getDevice(), *renderPass, temp };
        framebuffers.push_back(f);
    }
}

void recordCommandBuffer(vk::CommandBuffer commandBuffer, int imageIndex) {
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    commandBuffer.begin(beginInfo);

    vk::RenderPassBeginInfo renderPassInfo;

    renderPassInfo.renderPass = renderPass->getHandle();
    renderPassInfo.framebuffer = framebuffers[imageIndex]->getHandle();
    renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
    renderPassInfo.renderArea.extent = vk::Extent2D{ 1920, 1080 };


    vk::ClearColorValue color{ 0.0f, 0.0f, 0.0f, 1.0f };
    vk::ClearValue clearValue{ color };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValue;

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline->getHandle());
    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{ 0, 0 };
    scissor.extent = swapChainExtent;
    commandBuffer.setScissor(0, 1, &scissor);

    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRenderPass();

    commandBuffer.end();
}

void drawFrame()
{
    auto fence = gpuContext->requestFence();
    gpuContext->waitForFences(fence);
    auto imageAvailableSemaphore = gpuContext->requestSemaphore();
    auto renderFinishedSemaphore = gpuContext->requestSemaphore();

    auto acquieResult =
        gpuContext->acquireNextImage(imageAvailableSemaphore, VK_NULL_HANDLE);

    if (std::get<0>(acquieResult) == vk::Result::eErrorOutOfDateKHR)
    {
        recreateSwapChain();
        return;
    }
    else if (std::get<0>(acquieResult) != vk::Result::eSuccess && std::get<0>(acquieResult) != vk::Result::eSuboptimalKHR) { throw std::runtime_error("failed to acquire swap chain image!"); }

    gpuContext->resetFences(fence);

    commandPool->getCommandBuffer(currentFrame).reset(vk::CommandBufferResetFlagBits::eReleaseResources);
    recordCommandBuffer(static_cast<vk::CommandBuffer>(commandPool->getCommandBuffer(currentFrame)), std::get<1>(acquieResult));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { static_cast<VkSemaphore>(imageAvailableSemaphore) };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    auto cast = static_cast<VkCommandBuffer>(commandPool->getCommandBuffer(currentFrame));
    submitInfo.pCommandBuffers = &cast;

    VkSemaphore signalSemaphores[] = { static_cast<VkSemaphore>(renderFinishedSemaphore) };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(gpuContext->getDevice()->getGraphicsQueue(), 1, &submitInfo, fence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { gpuContext->getSwapchain()->getHandle() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &(std::get<1>(acquieResult));

    auto result = vkQueuePresentKHR(gpuContext->getDevice()->getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
    {
        framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS) { throw std::runtime_error("failed to present swap chain image!"); }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    gpuContext->returnSemaphore(renderFinishedSemaphore);
    gpuContext->returnSemaphore(imageAvailableSemaphore);

    gpuContext->returnFence(fence);
}

void cleanup() {
    cleanupSwapChain();
    delete graphicsPipeline;
    delete renderPass;
    delete commandPool;
}