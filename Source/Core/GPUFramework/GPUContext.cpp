#include "GPUContext.hpp"

#include "Common/Logging.hpp"

#include "Vulkan/Instance.hpp"
#include "Vulkan/Device.hpp"
#include "Vulkan/SemaphorePool.hpp"
#include "Vulkan/FencePool.hpp"
#include "Vulkan/Swapchain.hpp"
#include "Vulkan/ShaderModule.hpp"

#include "Platform/Window.hpp"


#include <filesystem>
#include <fstream>
#include <sstream>

GPUContext::GPUContext(const std::vector<const char*>& layers, const std::vector<const char*>& extensions, const std::shared_ptr<Window> window)
{
	instance = std::make_unique<Instance>("ToyEngine", extensions, layers);
    device = std::make_unique<Device>(*instance);
    // Deal with headless and normal rendering, only window rendering for now.
    if (window != nullptr) {
        window->createWindowSurface(this->instance->getHandle());
        this->window = window;
        swapchain = std::make_unique<Swapchain>(*device, static_cast<const vk::SurfaceKHR&>(window->getSurface()));
    }

    fencePool = std::make_unique<FencePool>(*device);
    semaphorePool = std::make_unique<SemaphorePool>(*device);
    loadShaders("C:/Users/lihan/Desktop/workspace/ToyEngine/Shader");
}

GPUContext::~GPUContext() 
{

}

const Device* GPUContext::getDevice() const 
{
    return this->device.get();
}

const std::vector<vk::Image>& GPUContext::getSwapchainImages() const
{
    return swapchain->getSwapchainImages();
}

const vk::Format GPUContext::getSwapchainFormat() const
{
    return swapchain->getFormat();
}

const vk::Extent2D GPUContext::getSwapchainExtent() const
{
    return swapchain->getExtent();
}

void GPUContext::rebuildSwapchainWithSize(const vk::Extent2D extent) const
{
    swapchain->rebuildWithSize(extent);
}

const std::tuple<vk::Result, uint32_t> GPUContext::acquireNextImage(const vk::Semaphore semaphore, const vk::Fence fence, uint32_t timeout) const
{
    auto result = device->getHandle().acquireNextImageKHR(swapchain->getHandle(), timeout, semaphore, fence);
    return std::tie(result.result, result.value);
}

const Swapchain* GPUContext::getSwapchain() const
{
    return swapchain.get();
}

const vk::Fence GPUContext::requestFence() const
{
    return fencePool->requestFence();
}

void GPUContext::waitForFences(const vk::Fence fence) const
{
    fencePool->waitForFences(fence);
}

void GPUContext::resetFences(const vk::Fence fence) const
{
    fencePool->resetFences(fence);
}

void GPUContext::returnFence(const vk::Fence fence) const
{
    fencePool->returnFence(fence);
}

const vk::Semaphore GPUContext::requestSemaphore() const
{
    return semaphorePool->requestSemaphore();
}

void GPUContext::returnSemaphore(const vk::Semaphore semaphore) const
{
    semaphorePool->returnSemaphore(semaphore);
}

const std::shared_ptr<ShaderModule> GPUContext::findShader(const std::string& name) const
{
    auto it = shaderModules.find(name);
    if (it != shaderModules.end()) {
        return it->second;
    }
    else {
        LOGE("ShaderModule{} not found", name);
        return nullptr;
    }
}

void GPUContext::loadShaders(const std::string& dir)
{
    for (const auto& entry: std::filesystem::directory_iterator(dir)) {
        if (entry.is_regular_file()) {
            std::ifstream file(entry.path(), std::ios::in | std::ios::binary);
            if (file) {
                std::ostringstream content;
                content << file.rdbuf();
                std::string extension = entry.path().extension().string();
                std::string name = entry.path().filename().string();
                if (!extension.compare(".vert")) {
                    shaderModules[name] = std::make_shared<ShaderModule>(*device, vk::ShaderStageFlagBits::eVertex, content.str());
                }
                else if (!extension.compare(".frag")) {
                    shaderModules[name] = std::make_shared<ShaderModule>(*device, vk::ShaderStageFlagBits::eFragment, content.str());
                }
                else if (!extension.compare(".comp")) {
                    shaderModules[name] = std::make_shared<ShaderModule>(*device, vk::ShaderStageFlagBits::eCompute, content.str());
                }
                else {
                    continue;
                }
            }
            else {
                LOGE("无法打开文件:{}", entry.path().string());
            }
        }
    }
}
