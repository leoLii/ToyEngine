#include "Taa.hpp"
#include "Core/GPUFramework/Vulkan/PipelineLayout.hpp"
#include "Core/GPUFramework/Vulkan/ComputePipeline.hpp"
#include "Core/GPUFramework/Vulkan/ShaderModule.hpp"
#include "Core/GPUFramework/Vulkan/DescriptorSetLayout.hpp"

#include <string>

TaaPass::TaaPass(const GPUContext* gpuContext, const Scene* scene)
	:gpuContext{gpuContext}
	,scene{scene}
{
	historyAttachment = new Attachment{};
}

TaaPass::~TaaPass() 
{
	gpuContext->destroyImage(historyAttachment->image);
	gpuContext->destroyImageView(historyAttachment->view);
	delete computePipeline;
	delete pipelineLayout;
	delete descriptorSet;
	delete descriptorSetLayout;
}

void TaaPass::prepare()
{
	initAttachment();

	std::vector<vk::DescriptorSetLayoutBinding> bindings;
	bindings.push_back(vk::DescriptorSetLayoutBinding{ 0, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute });
	bindings.push_back(vk::DescriptorSetLayoutBinding{ 1, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute });
	bindings.push_back(vk::DescriptorSetLayoutBinding{ 2, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute });
	bindings.push_back(vk::DescriptorSetLayoutBinding{ 3, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute });
	descriptorSetLayout = gpuContext->createDescriptorSetLayout(0, bindings);

	constants.stageFlags = vk::ShaderStageFlagBits::eCompute;
	constants.size = sizeof(Constant);
	constants.offset = 0;

	pipelineLayout = new PipelineLayout{ *gpuContext->getDevice(), {descriptorSetLayout->getHandle()}, {constants} };
	const ShaderModule* taaShader = gpuContext->findShader("taa.comp");
	computePipeline = new ComputePipeline{ *gpuContext->getDevice() , pipelineLayout, taaShader };

	std::unordered_map<uint32_t, vk::DescriptorImageInfo> imageInfos;
	imageInfos[0] = vk::DescriptorImageInfo{ VK_NULL_HANDLE, lightingResult->view->getHandle(), lightingResult->attachmentInfo.imageLayout };
	imageInfos[1] = vk::DescriptorImageInfo{ VK_NULL_HANDLE, historyAttachment->view->getHandle(), historyAttachment->attachmentInfo.imageLayout };
	imageInfos[2] = vk::DescriptorImageInfo{ VK_NULL_HANDLE, motionAttachment->view->getHandle(), motionAttachment->attachmentInfo.imageLayout };
	imageInfos[3] = vk::DescriptorImageInfo{ VK_NULL_HANDLE, depthAttachment->view->getHandle(), depthAttachment->attachmentInfo.imageLayout };

	descriptorSet = gpuContext->requireDescriptorSet(descriptorSetLayout, {}, imageInfos);
}

void TaaPass::update(uint32_t frameIndex)
{
}

void TaaPass::record(vk::CommandBuffer commandBuffer)
{
	commandBuffer.pushConstants<Constant>(
		pipelineLayout->getHandle(),
		vk::ShaderStageFlagBits::eCompute, 0,
		{ Constant{Vec2(960, 540)} });

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline->getHandle());
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout->getHandle(), 0, { descriptorSet->getHandle() }, {});
	commandBuffer.dispatch(60, 34, 1);
}

void TaaPass::setAttachment(uint32_t index, Attachment* attachment)
{
	switch (index)
	{
	case 0:
		lightingResult = attachment;
		break;
	case 1:
		motionAttachment = attachment;
		break;
	case 2:
		depthAttachment = attachment;
		break;
	default:
		break;
	}
}

Attachment* TaaPass::getAttachment()
{
	return lightingResult;
}

void TaaPass::initAttachment()
{
	ImageInfo imageInfo{};
	imageInfo.format = vk::Format::eR8G8B8A8Unorm;
	imageInfo.type = vk::ImageType::e2D;
	imageInfo.extent = vk::Extent3D{ width, height, 1 };
	imageInfo.usage =
		vk::ImageUsageFlagBits::eStorage;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.arrayLayers = 1;
	imageInfo.mipmapLevel = 1;
	imageInfo.queueFamilyCount = 1;
	imageInfo.pQueueFamilyIndices = { 0 };

	historyAttachment->image = gpuContext->createImage(imageInfo);
	historyAttachment->view = gpuContext->createImageView(historyAttachment->image);
	historyAttachment->format = imageInfo.format;
	historyAttachment->attachmentInfo.imageView = historyAttachment->view->getHandle();
	historyAttachment->attachmentInfo.imageLayout = vk::ImageLayout::eGeneral;
	historyAttachment->attachmentInfo.loadOp = vk::AttachmentLoadOp::eLoad;
	historyAttachment->attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
	historyAttachment->attachmentInfo.clearValue.color = vk::ClearColorValue{ 1.0f, 0.0f, 0.0f, 0.0f };
	historyAttachment->attachmentInfo.clearValue.depthStencil = vk::ClearDepthStencilValue{ 0u, 0u };

	auto commandBuffer = gpuContext->requestCommandBuffer(CommandType::Transfer, vk::CommandBufferLevel::ePrimary);
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	commandBuffer.begin(beginInfo);

	gpuContext->transferImage(
		commandBuffer,
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eMemoryWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
		historyAttachment->image);
	commandBuffer.end();

	gpuContext->submit(CommandType::Transfer, {}, {}, { commandBuffer }, {}, VK_NULL_HANDLE);

	gpuContext->getDevice()->getTransferQueue().waitIdle();
}
