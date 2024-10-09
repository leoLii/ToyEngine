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
#include "Core/GPUFramework/Vulkan/ImageView.hpp"

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
std::vector<std::shared_ptr<ImageView>> swapChainImageViews;
uint32_t currentFrame = 0;
bool framebufferResized = false;

void recreateSwapChain()
{
    int width = 0, height = 0;
    window->getFramebufferSize(&width, &height);
    while (width == 0 || height == 0)
    {
        window->getFramebufferSize(&width, &height);
        window->waitEvents();
    }

    gpuContext->getDevice()->getHandle().waitIdle();

    framebuffers.clear();
    swapChainImageViews.clear();

    gpuContext->rebuildSwapchainWithSize(vk::Extent2D(width, height));
    auto spImages = gpuContext->getSwapchainImages();
    swapChainImages.resize(spImages.size());

    std::transform(spImages.begin(), spImages.end(), swapChainImages.begin(), [](vk::Image i)->VkImage {
        return static_cast<VkImage>(i);
        });
    swapChainImageFormat = static_cast<VkFormat>(gpuContext->getSwapchainFormat());
    swapChainExtent = static_cast<VkExtent2D>(gpuContext->getSwapchainExtent());
    //swapchain->rebuildWithSize(vk::Extent2D(width, height));
    for (int i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews.push_back(gpuContext->createImageView(swapChainImages[i]));
    }

    for (int i = 0; i < swapChainImageViews.size(); i++) {
        std::vector<vk::ImageView> temp = { swapChainImageViews[i]->getHandle() };
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

    auto commandBuffer = commandPool->getCommandBuffer(currentFrame);
    commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
    recordCommandBuffer(static_cast<vk::CommandBuffer>(commandBuffer), std::get<1>(acquieResult));

    vk::SubmitInfo submitInfo;

    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

    gpuContext->getDevice()->getGraphicsQueue().submit(submitInfo, fence);

    vk::PresentInfoKHR presentInfo;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphore;

    auto swapChain = gpuContext->getSwapchain()->getHandle();
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain;

    presentInfo.pImageIndices = &(std::get<1>(acquieResult));

    auto result = gpuContext->getDevice()->getPresentQueue().presentKHR(presentInfo);

    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized)
    {
        framebufferResized = false;
        recreateSwapChain();
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    gpuContext->returnSemaphore(renderFinishedSemaphore);
    gpuContext->returnSemaphore(imageAvailableSemaphore);

    gpuContext->returnFence(fence);
}

void cleanup() {
    delete graphicsPipeline;
    delete renderPass;
    delete commandPool;
}