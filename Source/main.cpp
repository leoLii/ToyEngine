#include "Core/GPUFramework/Vulkan/Instance.hpp"
#include "Core/GPUFramework/Vulkan/Device.hpp"
#include "Core/GPUFramework/Vulkan/CommandPool.hpp"
#include "Core/GPUFramework/Vulkan/ImageView.h"
#include "Core/GPUFramework/Vulkan/Swapchain.hpp"
#include "Core/GPUFramework/Vulkan/RenderPass.hpp"
#include "Core/GPUFramework/Vulkan/GraphicsPipeline.hpp"
#include "Core/GPUFramework/Vulkan/SemaphorePool.hpp"
#include "Core/GPUFramework/Vulkan/FencePool.hpp"

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

const std::string vertexShaderText_PC_C = R"(
#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 0) uniform buffer
{
  mat4 mvp;
} uniformBuffer;

layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main()
{
  outColor = inColor;
  gl_Position = uniformBuffer.mvp * pos;
}
)";

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_portability_subset"};

int WIDTH = 1920;
int HEIGHT = 1080;

std::unique_ptr<Window> window;
Instance* instance;
Device* device;
Swapchain* swapchain;
CommandPool* commandPool;
RenderPass* renderPass;
GraphicsPipeline* graphicsPipeline;
static SemaphorePool* semaphorePool;
static FencePool* fencePool;

std::vector<VkFramebuffer> swapChainFramebuffers;


VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;

std::vector<VkImage> swapChainImages;
std::vector<VkImageView> swapChainImageViews;
uint32_t currentFrame = 0;
bool framebufferResized = false;


void cleanupSwapChain(){
    for (auto framebuffer : swapChainFramebuffers)
    {
        vkDestroyFramebuffer(device->getHandle(), framebuffer, nullptr);
    }

    for (auto imageView : swapChainImageViews)
    {
        vkDestroyImageView(device->getHandle(), imageView, nullptr);
    }
}

void createImageViews(){
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

        if (vkCreateImageView(device->getHandle(), &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

void createFramebuffers(){
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        VkImageView attachments[] = {swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass->getHandle();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device->getHandle(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
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

    vkDeviceWaitIdle(device->getHandle());

    cleanupSwapChain();
    swapchain->rebuildWithSize(vk::Extent2D(width, height));
    auto spImages = swapchain->getSwapchainImages();
    swapChainImages.resize(spImages.size());

    std::transform(spImages.begin(), spImages.end(), swapChainImages.begin(), [](vk::Image i)->VkImage {
        return static_cast<VkImage>(i);
        });
    swapChainImageFormat = static_cast<VkFormat>(swapchain->getFormat());
    swapChainExtent = static_cast<VkExtent2D>(swapchain->getExtent());
    //swapchain->rebuildWithSize(vk::Extent2D(width, height));
    createImageViews();
    createFramebuffers();
}

void recordCommandBuffer(VkCommandBuffer commandBuffer, int imageIndex){
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass->getHandle();
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->getHandle());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) { throw std::runtime_error("failed to record command buffer!"); }
}

void drawFrame()
{
    auto fence = static_cast<VkFence>(fencePool->requestFence());
    vkWaitForFences(device->getHandle(), 1, &fence, VK_TRUE, UINT64_MAX);
    auto imageAvailableSemaphore = semaphorePool->requestSemaphore();
    auto renderFinishedSemaphore = semaphorePool->requestSemaphore();
    uint32_t imageIndex;
    VkResult result =
    vkAcquireNextImageKHR(device->getHandle(), swapchain->getHandle(), UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { throw std::runtime_error("failed to acquire swap chain image!"); }

    vkResetFences(device->getHandle(), 1, &fence);

    vkResetCommandBuffer(commandPool->getCommandBuffer(currentFrame), /*VkCommandBufferResetFlagBits*/ 0);
    recordCommandBuffer(commandPool->getCommandBuffer(currentFrame), imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {static_cast<VkSemaphore>(imageAvailableSemaphore)};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    auto cast = static_cast<VkCommandBuffer>(commandPool->getCommandBuffer(currentFrame));
    submitInfo.pCommandBuffers = &cast;

    VkSemaphore signalSemaphores[] = {static_cast<VkSemaphore>(renderFinishedSemaphore)};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    if (vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, fence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapchain->getHandle()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(device->getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
    {
        framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS) { throw std::runtime_error("failed to present swap chain image!"); }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    semaphorePool->returnSemaphore(renderFinishedSemaphore);
    semaphorePool->returnSemaphore(imageAvailableSemaphore);

    fencePool->returnFence(fence);
}

int main() {

    window.reset(new Window("ToyEngine", WIDTH, HEIGHT));
    
    std::vector<const char *> extensions;
#ifdef ARCH_OS_MAC
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif // ARCH_OS_MAC
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for(int i=0; i<glfwExtensionCount;i++){
        extensions.push_back(glfwExtensions[i]);
    }
    
#ifdef ARCH_OS_MACOS
    instance = new Instance("ToyEngine", extensions, validationLayers, VK_API_VERSION_1_2);
#else
    instance = new Instance("ToyEngine", extensions, validationLayers);
#endif
    
    VkResult result = window->createWindowSurface(instance->getHandle());
    if (result != VK_SUCCESS)
    {
        std::runtime_error("Create Surface Error");
    }
    
    device = new Device(*instance);

    semaphorePool = new SemaphorePool(*device);
    fencePool = new FencePool(*device);

    auto surface = static_cast<vk::SurfaceKHR>(window->getSurface());
    swapchain = new Swapchain(*device, surface);

    auto spImages = swapchain->getSwapchainImages();
    swapChainImages.resize(spImages.size());

    std::transform(spImages.begin(), spImages.end(), swapChainImages.begin(), [](vk::Image i)->VkImage {
        return static_cast<VkImage>(i);
        });
    swapChainImageFormat = static_cast<VkFormat>(swapchain->getFormat());
    swapChainExtent = static_cast<VkExtent2D>(swapchain->getExtent());
    
    createImageViews();
    
    renderPass = new RenderPass(*device);
    
    auto readFile = [](const std::string &filename) -> std::vector<char>{
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) { throw std::runtime_error("failed to open file!"); }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    };
    
#ifdef ARCH_OS_MACOS
    auto vertShaderCode = readFile("/Users/leolii/Desktop/VulkanTest/VulkanTest/shaders/vert.spv");
    auto fragShaderCode = readFile("/Users/leolii/Desktop/VulkanTest/VulkanTest/shaders/frag.spv");
#endif // ARCH_OS_MACOS

    auto vertShaderCode = readFile("D:/Downloads/vert.spv");
    auto fragShaderCode = readFile("D:/Downloads/frag.spv");
    
    auto createShaderModule = [](const std::vector<char> &code) -> VkShaderModule
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device->getHandle(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    };

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
    std::vector<vk::ShaderModule> modules;
    modules.push_back(static_cast<vk::ShaderModule>(vertShaderModule));
    modules.push_back(static_cast<vk::ShaderModule>(fragShaderModule));
    graphicsPipeline = new GraphicsPipeline(*device, *renderPass, modules);

    vkDestroyShaderModule(device->getHandle(), fragShaderModule, nullptr);
    vkDestroyShaderModule(device->getHandle(), vertShaderModule, nullptr);
    
    createFramebuffers();
    
    commandPool = new CommandPool(*device, 0, MAX_FRAMES_IN_FLIGHT);
    
    while(!window->shouldClose()) {
        window->pollEvents();
        drawFrame();
    }

    return 0;
}
