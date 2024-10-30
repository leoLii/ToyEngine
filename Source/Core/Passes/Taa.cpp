#include "Taa.hpp"
#include "Core/GPUFramework/Vulkan/PipelineLayout.hpp"
#include "Core/GPUFramework/Vulkan/ComputePipeline.hpp"
#include "Core/GPUFramework/Vulkan/ShaderModule.hpp"
#include "Core/GPUFramework/Vulkan/DescriptorSetLayout.hpp"

#include <string>

TaaPass::TaaPass(const GPUContext* gpuContext, const Scene* scene, Vec2 size)
	:gpuContext{gpuContext}
	,scene{scene}
{
	taaOutput = new Attachment{};
	historyAttachment = new Attachment{};
	width = size.x;
	height = size.y;
}

TaaPass::~TaaPass() 
{
	gpuContext->destroyImage(historyAttachment->image);
	gpuContext->destroyImageView(historyAttachment->view);
	gpuContext->destroyImage(taaOutput->image);
	gpuContext->destroyImageView(taaOutput->view);
	gpuContext->destroySampler(sampler);
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
	bindings.push_back(vk::DescriptorSetLayoutBinding{ 2, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute });
	bindings.push_back(vk::DescriptorSetLayoutBinding{ 3, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute });
	bindings.push_back(vk::DescriptorSetLayoutBinding{ 4, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute });
	descriptorSetLayout = gpuContext->createDescriptorSetLayout(0, bindings);

	constants.stageFlags = vk::ShaderStageFlagBits::eCompute;
	constants.size = sizeof(Constant);
	constants.offset = 0;

	pipelineLayout = new PipelineLayout{ *gpuContext->getDevice(), {descriptorSetLayout->getHandle()}, {constants} };
	const ShaderModule* taaShader = gpuContext->findShader("taa.comp");
	computePipeline = new ComputePipeline{ *gpuContext->getDevice() , pipelineLayout, taaShader };

	sampler = gpuContext->createSampler();
	std::unordered_map<uint32_t, vk::DescriptorImageInfo> imageInfos;
	imageInfos[0] = vk::DescriptorImageInfo{ VK_NULL_HANDLE, taaOutput->view->getHandle(), taaOutput->attachmentInfo.imageLayout };
	imageInfos[1] = vk::DescriptorImageInfo{ VK_NULL_HANDLE, historyAttachment->view->getHandle(), historyAttachment->attachmentInfo.imageLayout };
	imageInfos[2] = vk::DescriptorImageInfo{ sampler, lightingResult->view->getHandle(), lightingResult->attachmentInfo.imageLayout };
	imageInfos[3] = vk::DescriptorImageInfo{ sampler, motionAttachment->view->getHandle(), motionAttachment->attachmentInfo.imageLayout };
	imageInfos[4] = vk::DescriptorImageInfo{ sampler, depthAttachment->view->getHandle(), depthAttachment->attachmentInfo.imageLayout };

	descriptorSet = gpuContext->requireDescriptorSet(descriptorSetLayout, {}, imageInfos);
}

void TaaPass::update(uint32_t frameIndex)
{
}

void TaaPass::record(vk::CommandBuffer commandBuffer)
{
	gpuContext->pipelineBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eComputeShader,
		vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eShaderRead,
		vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
		lightingResult->image);

	gpuContext->pipelineBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::PipelineStageFlagBits::eComputeShader,
		vk::AccessFlagBits::eDepthStencilAttachmentWrite, vk::AccessFlagBits::eShaderRead,
		vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
		depthAttachment->image,
		vk::DependencyFlagBits::eByRegion,
		vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });

	gpuContext->pipelineBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eComputeShader,
		vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eShaderRead,
		vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
		motionAttachment->image);

	commandBuffer.pushConstants<Constant>(
		pipelineLayout->getHandle(),
		vk::ShaderStageFlagBits::eCompute, 0,
		{ Constant{Vec2(width, height)} });

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline->getHandle());
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout->getHandle(), 0, { descriptorSet->getHandle() }, {});
	commandBuffer.dispatch(int((width + 16) / 16), int((height + 16) / 16), 1);
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
	return taaOutput;
}

void TaaPass::initAttachment()
{
	{
		ImageInfo imageInfo{};
		imageInfo.format = vk::Format::eR8G8B8A8Unorm;
		imageInfo.type = vk::ImageType::e2D;
		imageInfo.extent = vk::Extent3D{ width, height, 1 };
		imageInfo.usage =
			vk::ImageUsageFlagBits::eStorage |
			vk::ImageUsageFlagBits::eTransferSrc;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.arrayLayers = 1;
		imageInfo.mipmapLevel = 1;
		imageInfo.queueFamilyCount = 1;
		imageInfo.pQueueFamilyIndices = { 0 };

		taaOutput->image = gpuContext->createImage(imageInfo);
		taaOutput->view = gpuContext->createImageView(taaOutput->image);
		taaOutput->format = imageInfo.format;
		taaOutput->attachmentInfo.imageView = taaOutput->view->getHandle();
		taaOutput->attachmentInfo.imageLayout = vk::ImageLayout::eGeneral;
		taaOutput->attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
		taaOutput->attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
		taaOutput->attachmentInfo.clearValue = vk::ClearColorValue{ 0u, 0u, 0u, 0u };
	}

	{
		ImageInfo imageInfo{};
		imageInfo.format = vk::Format::eR8G8B8A8Unorm;
		imageInfo.type = vk::ImageType::e2D;
		imageInfo.extent = vk::Extent3D{ width, height, 1 };
		imageInfo.usage =
			vk::ImageUsageFlagBits::eStorage |
			vk::ImageUsageFlagBits::eTransferSrc;
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
		historyAttachment->attachmentInfo.clearValue = vk::ClearColorValue{ 0u, 0u, 0u, 0u };

	}
	
	auto commandBuffer = gpuContext->requestCommandBuffer(CommandType::Transfer, vk::CommandBufferLevel::ePrimary);
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	commandBuffer.begin(beginInfo);
	gpuContext->pipelineBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eMemoryWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
		taaOutput->image);
	gpuContext->pipelineBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eMemoryWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
		historyAttachment->image);
	commandBuffer.end();

	gpuContext->submit(CommandType::Transfer, {}, {}, { commandBuffer }, {}, VK_NULL_HANDLE);

	gpuContext->getDevice()->getTransferQueue().waitIdle();
}
