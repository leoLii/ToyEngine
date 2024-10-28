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

vk::CommandBuffer GPUContext::requestCommandBuffer(CommandType type, vk::CommandBufferLevel level) const
{
    return commandPools[0]->requestCommandBuffer(level);
}

PipelineLayout* GPUContext::createPipelineLayout(std::vector<vk::DescriptorSetLayout> setLayouts, std::vector<vk::PushConstantRange> constants) const
{
    return new PipelineLayout{ *device, setLayouts, constants };
}

GraphicsPipeline* GPUContext::createGraphicsPipeline(PipelineLayout* layout, GraphicsPipelineState* state, std::vector<const ShaderModule*> shaders) const
{
    return new GraphicsPipeline{ *device, layout, state, shaders };
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

const ShaderModule* GPUContext::findShader(const std::string& name) const
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

DescriptorSetLayout* GPUContext::createDescriptorSetLayout(uint32_t index, std::vector<vk::DescriptorSetLayoutBinding> bindings) const
{
    auto descriptorSetLayout = new DescriptorSetLayout{ *device, index, bindings };
    return descriptorSetLayout;
}

DescriptorSet* GPUContext::requireDescriptorSet(
    DescriptorSetLayout* layout,
    std::unordered_map<uint32_t, vk::DescriptorBufferInfo> bufferInfos,
    std::unordered_map<uint32_t, vk::DescriptorImageInfo> imageInfos) const
{
    return new DescriptorSet{ *device, layout, descriptorPool, bufferInfos, imageInfos };
}

ImageView* GPUContext::createImageView(
    Image* image, 
    vk::ImageViewType type,
    vk::ComponentMapping component, 
    vk::ImageSubresourceRange range) const
{
    return new ImageView(*device, image, type, image->getImageInfo().format, component, range);
}

void GPUContext::destroyImageView(ImageView* imageView) const
{
    delete imageView;
}

Buffer* GPUContext::createBuffer(uint64_t size, vk::BufferUsageFlags usage) const
{
    return new Buffer{ *device, size, usage };
}

void GPUContext::destroyBuffer(Buffer* buffer) const
{
    delete buffer;
}

Image* GPUContext::createImage(ImageInfo imageInfo) const
{
    return new Image{ *device, imageInfo };
}

void GPUContext::destroyImage(Image* image) const
{
    delete image;
}

vk::Sampler GPUContext::createSampler(
    vk::Filter magFilter, vk::Filter minFilter,
    vk::SamplerAddressMode addressMode,
    bool enableAnisotropy, float maxAnisotropy,
    vk::BorderColor borderColor) const
{
    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.magFilter = magFilter;  
    samplerInfo.minFilter = minFilter; 
    samplerInfo.addressModeU = addressMode; 
    samplerInfo.addressModeV = addressMode;
    samplerInfo.addressModeW = addressMode;
    samplerInfo.borderColor = borderColor;

    samplerInfo.anisotropyEnable = enableAnisotropy;
    samplerInfo.maxAnisotropy = enableAnisotropy ? maxAnisotropy : 1.0f;

    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f; 
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

    return device->getHandle().createSampler(samplerInfo);
}

void GPUContext::destroySampler(vk::Sampler sampler) const
{
    device->getHandle().destroySampler(sampler);
}

void GPUContext::submit(
    CommandType type,
    std::vector<vk::Semaphore> waitSemaphores,
    std::vector<vk::PipelineStageFlags> waitStages,
    std::vector<vk::CommandBuffer> commandBuffers,
    std::vector<vk::Semaphore> signalSemaphores,
    vk::Fence fence) const
{
    vk::SubmitInfo submitInfo;
    submitInfo.waitSemaphoreCount = waitSemaphores.size();
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();
    submitInfo.signalSemaphoreCount = signalSemaphores.size();
    submitInfo.pSignalSemaphores = signalSemaphores.data();

    switch (type)
    {
    case Graphics:
        device->getGraphicsQueue().submit(submitInfo, fence);
        break;
    case Compute:
        device->getComputeQueue().submit(submitInfo, fence);
        break;
    case Transfer:
        device->getTransferQueue().submit(submitInfo, fence);
        break;
    default:
        break;
    }
    
}

void GPUContext::present(uint32_t index, std::vector<vk::Semaphore> waitSemaphores)
{
    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = waitSemaphores.size();
    presentInfo.pWaitSemaphores = waitSemaphores.data();

    presentInfo.swapchainCount = 1;
    auto swapchainHandle = swapchain->getHandle();
    presentInfo.pSwapchains = &swapchainHandle;
    presentInfo.pImageIndices = &index;

    auto result = device->getPresentQueue().presentKHR(presentInfo);
    if (result != vk::Result::eSuccess) {
        std::runtime_error("Error present");
    }
}

void GPUContext::transferImage(
    vk::CommandBuffer commandBuffer,
    vk::PipelineStageFlags srcStage, vk::PipelineStageFlags dstStage,
    vk::AccessFlags srcAccess, vk::AccessFlags dstAccess, 
    vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
    const Image* image, 
    vk::DependencyFlags dependencyFlags,
    vk::ImageSubresourceRange range, 
    uint32_t srcFamily, uint32_t dstFamily) const
{
    vk::ImageMemoryBarrier imageMemoryBarrier;
    imageMemoryBarrier.srcAccessMask = srcAccess;
    imageMemoryBarrier.dstAccessMask = dstAccess;
    imageMemoryBarrier.oldLayout = oldLayout;
    imageMemoryBarrier.newLayout = newLayout;
    imageMemoryBarrier.image = image->getHandle();
    imageMemoryBarrier.subresourceRange = range;
    imageMemoryBarrier.srcQueueFamilyIndex = srcFamily;
    imageMemoryBarrier.dstQueueFamilyIndex = dstFamily;
    commandBuffer.pipelineBarrier(
        srcStage,
        dstStage,
        dependencyFlags,
        0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
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
                    shaderModules[name] = new ShaderModule(*device, vk::ShaderStageFlagBits::eVertex, content.str());
                }
                else if (!extension.compare(".frag")) {
                    shaderModules[name] = new ShaderModule(*device, vk::ShaderStageFlagBits::eFragment, content.str());
                }
                else if (!extension.compare(".comp")) {
                    shaderModules[name] = new ShaderModule(*device, vk::ShaderStageFlagBits::eCompute, content.str());
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
        delete shader.second;
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
