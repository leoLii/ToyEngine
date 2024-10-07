#include "GPUContext.hpp"

#include "Vulkan/Instance.hpp"
#include "Vulkan/Device.hpp"
#include "Vulkan/SemaphorePool.hpp"
#include "Vulkan/FencePool.hpp"
#include "Vulkan/Swapchain.hpp"

#include "Platform/Window.hpp"

GPUContext::GPUContext(const std::vector<const char*>& layers, const std::vector<const char*>& extensions, const std::shared_ptr<Window> window)
{
	instance = std::make_unique<Instance>("ToyEngine", extensions, layers);
    device = std::make_unique<Device>(*instance);
    fencePool = std::make_unique<FencePool>(*device);
    semaphorePool = std::make_unique<SemaphorePool>(*device);
    // Deal with headless and normal rendering, only window rendering for now.
    if (window != nullptr) {
        window->createWindowSurface(this->instance->getHandle());
        this->window = window;
        swapchain = std::make_unique<Swapchain>(*device, static_cast<vk::SurfaceKHR&>(window->getSurface()));
    }
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





