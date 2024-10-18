#include "GPUContext.hpp"

#include "Common/Logging.hpp"

#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>

GPUContext::GPUContext(
    const std::string name,
    const std::vector<const char*>& layers,
    const std::vector<const char*>& extensions,
    Window* window)
{
    this->vulkanExtensions = extensions;
    this->vulkanLayers = layers;
	instance = std::make_unique<Instance>(name, extensions, layers);
    device = std::make_unique<Device>(*instance);
    // Deal with headless and normal rendering, only window rendering for now.
    if (window != nullptr) {
        createSurface(window);
        swapchain = std::make_unique<Swapchain>(*device, static_cast<const vk::SurfaceKHR&>(this->surface));
    }

    fencePool = std::make_unique<FencePool>(*device);
    semaphorePool = std::make_unique<SemaphorePool>(*device);
    loadShaders("C:/Users/lihan/Desktop/workspace/ToyEngine/Shader");
    createDescriptorPool();
    createCommandPools();
}

GPUContext::~GPUContext() 
{
    destroyCommandPools();
    destroyDescriptorPool();
    destroyShaders();
    swapchain.reset();
    destroySurface();
}

const Device* GPUContext::getDevice() const 
{
    return this->device.get();
}

const std::vector<Image*>& GPUContext::getSwapchainImages() const
{
    return swapchain->getImages();
}

const std::vector<ImageView*>& GPUContext::getSwapchainImageViews() const
{
    return swapchain->getImageViews();
}

uint32_t GPUContext::getSwapchainImageCount() const
{
    return swapchain->getImageCount();
}

vk::Format GPUContext::getSwapchainFormat() const
{
    return swapchain->getSwapchainImageInfo().format;
}

vk::Extent2D GPUContext::getSwapchainExtent() const
{
    auto extent = swapchain->getSwapchainImageInfo().extent;
    return vk::Extent2D(extent.width, extent.height);
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

vk::SurfaceKHR GPUContext::getSurface() const
{
    return surface;
}

vk::CommandBuffer GPUContext::requestCommandBuffer(vk::CommandBufferLevel level)
{
    return commandPools[0]->requestCommandBuffer(level);
}

vk::Fence GPUContext::requestFence() const
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

vk::Semaphore GPUContext::requestSemaphore() const
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

std::vector<vk::DescriptorSet> GPUContext::requireDescriptorSet(std::vector<vk::DescriptorSetLayout> layouts)
{
    vk::DescriptorSetAllocateInfo allocateInfo;
    allocateInfo.descriptorPool = descriptorPool;
    allocateInfo.descriptorSetCount = layouts.size();
    allocateInfo.pSetLayouts = layouts.data();
    auto result = device->getHandle().allocateDescriptorSets(allocateInfo);
    return result;
}

const std::shared_ptr<ImageView> GPUContext::createImageView(
    const vk::Image image, 
    const vk::ImageViewType type, 
    const vk::Format format, 
    const vk::ComponentMapping component, 
    const vk::ImageSubresourceRange range)
{
    auto imageView = std::make_shared<ImageView>(*device, image, type, format, component, range);
    imageViews.push_back(imageView);
    return imageView;
}

Buffer* GPUContext::createBuffer(uint64_t size, vk::BufferUsageFlags usage)
{
    Buffer* buffer = new Buffer{ *device, size, usage };
    return buffer;
}

void GPUContext::destroyBuffer(Buffer* buffer)
{
    delete buffer;
}

void GPUContext::createCommandPools()
{
    commandPools.push_back(new CommandPool{ *device, 0 });
}

void GPUContext::destroyCommandPools()
{
    for (auto commandPool : commandPools) {
        delete commandPool;
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

void GPUContext::destroyShaders()
{
    for (auto shader : shaderModules) {
        shader.second.reset();
    }
    shaderModules.clear();
}

void GPUContext::createSurface(Window* window)
{
    VkSurfaceKHR VKSurface;
    auto result = glfwCreateWindowSurface(instance->getHandle(), window->getHandle(), nullptr, &VKSurface);
    if (result != VK_SUCCESS) {
        throw VulkanException(static_cast<vk::Result>(result));
    }
    surface = static_cast<vk::SurfaceKHR>(VKSurface);
}

void GPUContext::destroySurface()
{
    instance->getHandle().destroySurfaceKHR(surface);
}

void GPUContext::createDescriptorPool()
{
    std::array<vk::DescriptorPoolSize, 4> poolSize;
    poolSize[0].type = vk::DescriptorType::eUniformBuffer;
    poolSize[0].descriptorCount = 64;
    poolSize[1].type = vk::DescriptorType::eCombinedImageSampler;
    poolSize[1].descriptorCount = 64;
    poolSize[2].type = vk::DescriptorType::eStorageBuffer;
    poolSize[2].descriptorCount = 64;
    poolSize[3].type = vk::DescriptorType::eStorageImage;
    poolSize[3].descriptorCount = 64;
    vk::DescriptorPoolCreateInfo createInfo;
    createInfo.maxSets = 10000;
    createInfo.poolSizeCount = poolSize.size();
    createInfo.pPoolSizes = poolSize.data();

    descriptorPool = device->getHandle().createDescriptorPool(createInfo);
}

void GPUContext::destroyDescriptorPool()
{
    device->getHandle().destroyDescriptorPool(descriptorPool);
}
