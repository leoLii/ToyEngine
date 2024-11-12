#include "Taa.hpp"

#include "Scene/Scene.hpp"
#include "Core/GPUFramework/Vulkan/PipelineLayout.hpp"
#include "Core/GPUFramework/Vulkan/ComputePipeline.hpp"
#include "Core/GPUFramework/Vulkan/ShaderModule.hpp"
#include "Core/GPUFramework/Vulkan/DescriptorSetLayout.hpp"

#include <string>

TaaPass::TaaPass(const GPUContext* gpuContext, ResourceManager* resourceManager, const Scene* scene, Vec2 size)
	:gpuContext{gpuContext}
	, resourceManager{ resourceManager }
	,scene{scene}
{
	width = size.x;
	height = size.y;
	initAttachment();
}

TaaPass::~TaaPass() 
{
	delete computePipeline;
	delete pipelineLayout;
	delete descriptorSet;
	delete descriptorSetLayout;
}

void TaaPass::prepare()
{
	linearSampler = resourceManager->createSampler();
	nearestSampler = resourceManager->createSampler(vk::Filter::eNearest, vk::Filter::eNearest);

	std::vector<vk::DescriptorSetLayoutBinding> bindings;
	bindings.push_back(vk::DescriptorSetLayoutBinding{ 0, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute });
	bindings.push_back(vk::DescriptorSetLayoutBinding{ 1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute, &linearSampler });
	bindings.push_back(vk::DescriptorSetLayoutBinding{ 2, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute, &linearSampler });
	bindings.push_back(vk::DescriptorSetLayoutBinding{ 3, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute, &nearestSampler });
	bindings.push_back(vk::DescriptorSetLayoutBinding{ 4, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute, &nearestSampler });
	descriptorSetLayout = gpuContext->createDescriptorSetLayout(0, bindings);

	constants.stageFlags = vk::ShaderStageFlagBits::eCompute;
	constants.size = sizeof(Constant);
	constants.offset = 0;

	pipelineLayout = new PipelineLayout{ *gpuContext->getDevice(), {descriptorSetLayout->getHandle()}, {constants} };
	const ShaderModule* taaShader = gpuContext->findShader("taa.comp");
	computePipeline = new ComputePipeline{ *gpuContext->getDevice() , pipelineLayout, taaShader };

	std::unordered_map<uint32_t, vk::DescriptorImageInfo> imageInfos;
	imageInfos[0] = vk::DescriptorImageInfo{ VK_NULL_HANDLE, taaOutput->view->getHandle(), taaOutput->attachmentInfo.layout };
	imageInfos[1] = vk::DescriptorImageInfo{ VK_NULL_HANDLE, history->view->getHandle(), history->attachmentInfo.layout };
	imageInfos[2] = vk::DescriptorImageInfo{ VK_NULL_HANDLE, lightingResult->view->getHandle(), lightingResult->attachmentInfo.layout };
	imageInfos[3] = vk::DescriptorImageInfo{ VK_NULL_HANDLE, velocity->view->getHandle(), velocity->attachmentInfo.layout };
	imageInfos[4] = vk::DescriptorImageInfo{ VK_NULL_HANDLE, depth->view->getHandle(), depth->attachmentInfo.layout };

	descriptorSet = gpuContext->requireDescriptorSet(descriptorSetLayout, {}, imageInfos);
}

void TaaPass::update(uint32_t frameIndex)
{
}

void TaaPass::record(vk::CommandBuffer commandBuffer)
{
	gpuContext->imageBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits2::eTransfer, vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eTransferWrite, vk::AccessFlagBits2::eShaderRead,
		vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
		history->image);

	gpuContext->imageBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2::eShaderRead,
		vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
		lightingResult->image);

	gpuContext->imageBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2::eShaderRead,
		vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
		velocity->image);

	gpuContext->imageBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests, vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite, vk::AccessFlagBits2::eShaderRead,
		vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
		depth->image,
		vk::DependencyFlagBits::eByRegion,
		vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });

	auto camera = scene->getCamera();
	commandBuffer.pushConstants<Constant>(
		pipelineLayout->getHandle(),
		vk::ShaderStageFlagBits::eCompute, 0,
		{ Constant{Vec2(width, height), camera->getCurrJitter()}});

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline->getHandle());
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout->getHandle(), 0, { descriptorSet->getHandle() }, {});
	commandBuffer.dispatch(int((width + 16) / 16), int((height + 16) / 16), 1);
	gpuContext->pipelineBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eTransfer,
		vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eTransferRead,
		vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
		taaOutput->image);

	gpuContext->pipelineBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eTransfer,
		vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eTransferWrite,
		vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
		history->image);

	vk::ImageCopy copyRegion;
	copyRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	copyRegion.srcSubresource.mipLevel = 0;
	copyRegion.srcSubresource.baseArrayLayer = 0;
	copyRegion.srcSubresource.layerCount = 1;
	copyRegion.srcOffset = vk::Offset3D{ 0, 0, 0 };

	copyRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	copyRegion.dstSubresource.mipLevel = 0;
	copyRegion.dstSubresource.baseArrayLayer = 0;
	copyRegion.dstSubresource.layerCount = 1;
	copyRegion.dstOffset = vk::Offset3D{ 0, 0, 0 };

	copyRegion.extent.width = width;
	copyRegion.extent.height = height;
	copyRegion.extent.depth = 1;

	commandBuffer.copyImage(
		taaOutput->image->getHandle(), vk::ImageLayout::eGeneral,
		history->image->getHandle(), vk::ImageLayout::eGeneral, { copyRegion });
}

void TaaPass::end()
{
}

void TaaPass::initAttachment()
{
	lightingResult = resourceManager->getAttachment("ColorBuffer");
	velocity = resourceManager->getAttachment("gVelocity");
	depth = resourceManager->getAttachment("gDepth");

	taaOutput = resourceManager->getAttachment("taaOutput");
	history = resourceManager->getAttachment("taaHistory");
	
	auto commandBuffer = gpuContext->requestCommandBuffer(CommandType::Transfer, vk::CommandBufferLevel::ePrimary);
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	commandBuffer.begin(beginInfo);

	gpuContext->pipelineBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eShaderRead,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
		history->image);

	gpuContext->pipelineBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eMemoryWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
		taaOutput->image);
	
	commandBuffer.end();

	gpuContext->submit(CommandType::Transfer, {}, {}, { commandBuffer }, {}, VK_NULL_HANDLE);

	gpuContext->getDevice()->getTransferQueue().waitIdle();
}
