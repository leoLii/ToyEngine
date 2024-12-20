#include "GPUContext.hpp"

#include "Common/Logging.hpp"

#include <vector>

GPUContext::~GPUContext()
{
	destroyCommandPools();
	destroyDescriptorPool();
	swapchain.reset();
	destroySurface();
}

void GPUContext::init(
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
	createDescriptorPool();
	createCommandPools();
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

vk::CommandBuffer GPUContext::requestCommandBuffer(CommandType type, vk::CommandBufferLevel level, uint32_t poolIndex) const
{
	return commandPools[poolIndex][type]->requestCommandBuffer(level);
}

vk::CommandPool GPUContext::getCommandPool(uint32_t poolIndex, QueueType type) const
{
	return commandPools[poolIndex][type]->getHandle();
}

vk::CommandPool GPUContext::getTextureLoadPool() const
{
	return textureLoadPool->getHandle();
}

PipelineLayout* GPUContext::createPipelineLayout(std::vector<vk::DescriptorSetLayout> setLayouts, std::vector<vk::PushConstantRange> constants) const
{
	return new PipelineLayout{ *device, setLayouts, constants };
}

GraphicsPipeline* GPUContext::createGraphicsPipeline(PipelineLayout* layout, vk::PipelineCache cache, GraphicsPipelineState* state, std::vector<const ShaderModule*> shaders) const
{
	return new GraphicsPipeline{ *device, layout, cache, state, shaders };
}

ComputePipeline* GPUContext::createComputePipeline(PipelineLayout* layout, vk::PipelineCache cache, const ShaderModule* shader) const
{
	return new ComputePipeline{ *device, layout, cache, shader };
}

vk::Fence GPUContext::requestFence() const
{
	return fencePool->requestFence();
}

void GPUContext::waitForFences(std::vector<vk::Fence> fences) const
{
	fencePool->waitForFences(fences);
}

void GPUContext::resetFences(std::vector<vk::Fence> fences) const
{
	fencePool->resetFences(fences);
}

void GPUContext::returnFence(vk::Fence fence) const
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
	case cGraphics:
		std::get<1>(device->getQueue(QueueType::qGraphics)).submit(submitInfo, fence);
		break;
	case cCompute:
		std::get<1>(device->getQueue(QueueType::qCompute)).submit(submitInfo, fence);
		break;
	case cTransfer:
		std::get<1>(device->getQueue(QueueType::qTransfer)).submit(submitInfo, fence);
		break;
	default:
		break;
	}

}

void GPUContext::present(uint32_t index, std::vector<vk::Semaphore> waitSemaphores) const
{
	vk::PresentInfoKHR presentInfo{};
	presentInfo.waitSemaphoreCount = waitSemaphores.size();
	presentInfo.pWaitSemaphores = waitSemaphores.data();

	presentInfo.swapchainCount = 1;
	auto swapchainHandle = swapchain->getHandle();
	presentInfo.pSwapchains = &swapchainHandle;
	presentInfo.pImageIndices = &index;

	auto result = std::get<1>(device->getQueue(QueueType::qPresent)).presentKHR(presentInfo);
	if (result != vk::Result::eSuccess) {
		std::runtime_error("Error present");
	}
}

void GPUContext::pipelineBarrier(
	vk::CommandBuffer commandBuffer,
	vk::PipelineStageFlags srcStage, vk::PipelineStageFlags dstStage,
	vk::AccessFlags srcAccess, vk::AccessFlags dstAccess,
	vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
	const Image* image,
	vk::DependencyFlags dependencyFlags,
	vk::ImageSubresourceRange range,
	uint32_t srcFamily, uint32_t dstFamily) const
{
	vk::ImageMemoryBarrier imageMemoryBarrier{};
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

void GPUContext::imageBarrier(
	vk::CommandBuffer commandBuffer,
	vk::PipelineStageFlags2 srcStage, vk::AccessFlags2 srcAccess,
	vk::PipelineStageFlags2 dstStage, vk::AccessFlags2 dstAccess,
	vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
	const Image* image,
	vk::DependencyFlags dependencyFlags,
	vk::ImageSubresourceRange range,
	uint32_t srcFamily, uint32_t dstFamily) const
{
	vk::ImageMemoryBarrier2 imageMemoryBarrier{};
	imageMemoryBarrier.srcStageMask = srcStage;
	imageMemoryBarrier.dstStageMask = dstStage;
	imageMemoryBarrier.srcAccessMask = srcAccess;
	imageMemoryBarrier.dstAccessMask = dstAccess;
	imageMemoryBarrier.oldLayout = oldLayout;
	imageMemoryBarrier.newLayout = newLayout;
	imageMemoryBarrier.image = image->getHandle();
	imageMemoryBarrier.subresourceRange = range;
	imageMemoryBarrier.srcQueueFamilyIndex = srcFamily;
	imageMemoryBarrier.dstQueueFamilyIndex = dstFamily;

	vk::DependencyInfo dependencyInfo{};
	dependencyInfo.dependencyFlags = vk::DependencyFlagBits::eByRegion;
	dependencyInfo.imageMemoryBarrierCount = 1;
	dependencyInfo.pImageMemoryBarriers = &imageMemoryBarrier;

	commandBuffer.pipelineBarrier2(dependencyInfo);
}

void GPUContext::bufferBarrier(vk::CommandBuffer commandBuffer,
	vk::PipelineStageFlags2 srcStage, vk::AccessFlags2 srcAccess,
	vk::PipelineStageFlags2 dstStage, vk::AccessFlags2 dstAccess,
	const Buffer* buffer,
	uint32_t srcFamily, uint32_t dstFamily
) const
{
	vk::BufferMemoryBarrier2 bufferMemoryBarrier{};
	bufferMemoryBarrier.srcStageMask = srcStage;
	bufferMemoryBarrier.dstStageMask = dstStage;
	bufferMemoryBarrier.srcAccessMask = srcAccess;
	bufferMemoryBarrier.dstAccessMask = dstAccess;
	bufferMemoryBarrier.buffer = buffer->getHandle();
	bufferMemoryBarrier.offset = buffer->getOffset();
	bufferMemoryBarrier.size = buffer->getSize();

	vk::DependencyInfo dependencyInfo{};
	dependencyInfo.dependencyFlags = vk::DependencyFlagBits::eByRegion;
	dependencyInfo.bufferMemoryBarrierCount = 1;
	dependencyInfo.pBufferMemoryBarriers = &bufferMemoryBarrier;

	commandBuffer.pipelineBarrier2(dependencyInfo);
}

void GPUContext::createCommandPools()
{
	commandPools.push_back({
		new CommandPool{ *device, std::get<0>(device->getQueue(QueueType::qGraphics)) },
		new CommandPool{ *device, std::get<0>(device->getQueue(QueueType::qCompute)) },
		new CommandPool{ *device, std::get<0>(device->getQueue(QueueType::qTransfer)) } });
	commandPools.push_back({
		new CommandPool{ *device, std::get<0>(device->getQueue(QueueType::qGraphics)) },
		new CommandPool{ *device, std::get<0>(device->getQueue(QueueType::qCompute)) },
		new CommandPool{ *device, std::get<0>(device->getQueue(QueueType::qTransfer)) } });

	textureLoadPool = new CommandPool{ *device, 0 };
}

void GPUContext::destroyCommandPools()
{
	for (auto pools : commandPools) {
		for (auto commandPool : pools) {
			delete commandPool;
		}
	}
	delete textureLoadPool;
}

void GPUContext::createSurface(Window* window)
{
	VkSurfaceKHR VKSurface;
	auto result = SDL_Vulkan_CreateSurface(window->getHandle(), instance->getHandle(), &VKSurface);
	if (result != SDL_TRUE) {
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
