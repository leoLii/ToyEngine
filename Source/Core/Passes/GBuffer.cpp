#include "GBuffer.hpp"

GBufferPass::GBufferPass(const GPUContext* context, ResourceManager* resourceManager, const Scene* scene, Vec2 size)
	:gpuContext{ context }
	, resourceManager{ resourceManager }
	, scene{ scene }
{
	width = size.x;
	height = size.y;
	initAttachments();
}

GBufferPass::~GBufferPass()
{
	delete graphicsPipeline;
	delete pipelineLayout;
	delete descriptorSetLayout;
	delete descriptorSet;
	//delete pipelineState;
}

void GBufferPass::initAttachments()
{
	{
		positionAttachment = resourceManager->getAttachment("gPosition");
		vk::RenderingAttachmentInfo attachmentInfo{};
		attachmentInfo.imageView = positionAttachment->view->getHandle();
		attachmentInfo.imageLayout = positionAttachment->attachmentInfo.layout;
		attachmentInfo.loadOp = positionAttachment->attachmentInfo.loadOp;
		attachmentInfo.storeOp = positionAttachment->attachmentInfo.storeOp;
		attachmentInfo.clearValue = positionAttachment->attachmentInfo.clearValue;
		renderingAttachments.push_back(attachmentInfo);
		attachmentFormats.push_back(positionAttachment->attachmentInfo.format);
	}
	{
		albedoAttachment = resourceManager->getAttachment("gAlbedo");
		vk::RenderingAttachmentInfo attachmentInfo{};
		attachmentInfo.imageView = albedoAttachment->view->getHandle();
		attachmentInfo.imageLayout = albedoAttachment->attachmentInfo.layout;
		attachmentInfo.loadOp = albedoAttachment->attachmentInfo.loadOp;
		attachmentInfo.storeOp = albedoAttachment->attachmentInfo.storeOp;
		attachmentInfo.clearValue = albedoAttachment->attachmentInfo.clearValue;
		renderingAttachments.push_back(attachmentInfo);
		attachmentFormats.push_back(albedoAttachment->attachmentInfo.format);
	}
	{
		normalAttachment = resourceManager->getAttachment("gNormal");
		vk::RenderingAttachmentInfo attachmentInfo{};
		attachmentInfo.imageView = normalAttachment->view->getHandle();
		attachmentInfo.imageLayout = normalAttachment->attachmentInfo.layout;
		attachmentInfo.loadOp = normalAttachment->attachmentInfo.loadOp;
		attachmentInfo.storeOp = normalAttachment->attachmentInfo.storeOp;
		attachmentInfo.clearValue = normalAttachment->attachmentInfo.clearValue;
		renderingAttachments.push_back(attachmentInfo);
		attachmentFormats.push_back(normalAttachment->attachmentInfo.format);
	}
	{
		armAttachment = resourceManager->getAttachment("gARM");
		vk::RenderingAttachmentInfo attachmentInfo{};
		attachmentInfo.imageView = armAttachment->view->getHandle();
		attachmentInfo.imageLayout = armAttachment->attachmentInfo.layout;
		attachmentInfo.loadOp = armAttachment->attachmentInfo.loadOp;
		attachmentInfo.storeOp = armAttachment->attachmentInfo.storeOp;
		attachmentInfo.clearValue = armAttachment->attachmentInfo.clearValue;
		renderingAttachments.push_back(attachmentInfo);
		attachmentFormats.push_back(armAttachment->attachmentInfo.format);
	}
	{
		velocityAttachment = resourceManager->getAttachment("gVelocity");
		vk::RenderingAttachmentInfo attachmentInfo{};
		attachmentInfo.imageView = velocityAttachment->view->getHandle();
		attachmentInfo.imageLayout = velocityAttachment->attachmentInfo.layout;
		attachmentInfo.loadOp = velocityAttachment->attachmentInfo.loadOp;
		attachmentInfo.storeOp = velocityAttachment->attachmentInfo.storeOp;
		attachmentInfo.clearValue = velocityAttachment->attachmentInfo.clearValue;
		renderingAttachments.push_back(attachmentInfo);
		attachmentFormats.push_back(velocityAttachment->attachmentInfo.format);
	}

	{
		depthAttachment = resourceManager->getAttachment("gDepth");
		depthAttachmentInfo.imageView = depthAttachment->view->getHandle();
		depthAttachmentInfo.imageLayout = depthAttachment->attachmentInfo.layout;
		depthAttachmentInfo.loadOp = depthAttachment->attachmentInfo.loadOp;
		depthAttachmentInfo.storeOp = depthAttachment->attachmentInfo.storeOp;
		depthAttachmentInfo.clearValue = depthAttachment->attachmentInfo.clearValue;
	}

	auto commandBuffer = gpuContext->requestCommandBuffer(CommandType::Transfer, vk::CommandBufferLevel::ePrimary);
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	commandBuffer.begin(beginInfo);

	gpuContext->imageBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
		positionAttachment->image);

	gpuContext->imageBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
		albedoAttachment->image);

	gpuContext->imageBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
		normalAttachment->image);

	gpuContext->imageBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
		armAttachment->image);

	gpuContext->imageBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
		velocityAttachment->image);

	gpuContext->imageBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eAllGraphics,
		vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
		depthAttachment->image, vk::DependencyFlagBits::eByRegion,
		vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });

	commandBuffer.end();

	gpuContext->submit(CommandType::Transfer, {}, {}, { commandBuffer }, {}, VK_NULL_HANDLE);
	
	gpuContext->getDevice()->getTransferQueue().waitIdle();
}

void GBufferPass::prepare()
{
	renderingInfo.layerCount = 1;
	renderingInfo.renderArea.offset = vk::Offset2D{};
	renderingInfo.renderArea.extent = vk::Extent2D{ width, height };
	renderingInfo.colorAttachmentCount = renderingAttachments.size();
	renderingInfo.pColorAttachments = renderingAttachments.data();
	renderingInfo.pDepthAttachment = &depthAttachmentInfo;

	viewport.x = 0.0f;
	viewport.y = height;
	viewport.width = width;
	viewport.height = -int(height);
	viewport.minDepth = 1.0f;
	viewport.maxDepth = 0.0f;

	scissor.offset = vk::Offset2D{ 0, 0 };
	scissor.extent = vk::Extent2D{ width, height };

	std::vector<const ShaderModule*> baseModules = { gpuContext->findShader("gbuffer.vert"), gpuContext->findShader("gbuffer.frag") };
	vk::DescriptorSetLayoutBinding binding = vk::DescriptorSetLayoutBinding{ 0, vk::DescriptorType::eUniformBufferDynamic, 1, vk::ShaderStageFlagBits::eVertex };
	descriptorSetLayout = gpuContext->createDescriptorSetLayout(0, { binding });

	constants.stageFlags = vk::ShaderStageFlagBits::eVertex;
	constants.size = sizeof(Constant);
	constants.offset = 0;

	pipelineLayout = new PipelineLayout{ *gpuContext->getDevice(), { descriptorSetLayout->getHandle() }, {constants} };

	std::vector<vk::VertexInputBindingDescription> vertexBindings;
	vertexBindings.push_back(
		vk::VertexInputBindingDescription{ 0, sizeof(Vertex), vk::VertexInputRate::eVertex });

	std::vector<vk::VertexInputAttributeDescription> vertexAttributes;
	// Position
	vertexAttributes.push_back(
		vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)));
	// Texcoord
	vertexAttributes.push_back(
		vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texcoord)));
	// Normal
	vertexAttributes.push_back(
		vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)));
	// Tangent
	vertexAttributes.push_back(
		vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, tangent)));

	GraphicsPipelineState state;
	state.vertexInputState.bindings = vertexBindings;
	state.vertexInputState.attributes = vertexAttributes;
	state.dynamicStates.push_back(vk::DynamicState::eViewport);
	state.dynamicStates.push_back(vk::DynamicState::eScissor);
	state.renderingInfo.colorAttachmentFormats = attachmentFormats;

	graphicsPipeline = new GraphicsPipeline(*gpuContext->getDevice(), pipelineLayout, &state, baseModules);

	uniformBuffer = resourceManager->createBuffer(sizeof(Uniform) * scene->getMeshCount(), vk::BufferUsageFlagBits::eUniformBuffer);
	
	vk::DescriptorBufferInfo descriptorBufferInfo;
	descriptorBufferInfo.buffer = uniformBuffer->getHandle();
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = sizeof(Uniform);

	std::unordered_map<uint32_t, vk::DescriptorBufferInfo> bufferInfos = { {0, descriptorBufferInfo} };
	std::unordered_map<uint32_t, vk::DescriptorImageInfo> imageInfos;
	descriptorSet = gpuContext->requireDescriptorSet(descriptorSetLayout, bufferInfos, imageInfos);

	auto vertices = scene->getVertices();
	vertexBuffer = resourceManager->createBuffer(vertices.size() * sizeof(Vertex), vk::BufferUsageFlagBits::eVertexBuffer);
	vertexBuffer->copyToGPU(static_cast<const void*>(vertices.data()), vertices.size() * sizeof(Vertex));

	auto indices = scene->getIndices();
	indexBuffer = resourceManager->createBuffer(indices.size() * sizeof(uint32_t), vk::BufferUsageFlagBits::eIndexBuffer);
	indexBuffer->copyToGPU(static_cast<const void*>(indices.data()), indices.size() * sizeof(uint32_t));
}

void GBufferPass::record(vk::CommandBuffer commandBuffer)
{
	auto camera = scene->getCamera();

	commandBuffer.beginRendering(&renderingInfo);

	commandBuffer.pushConstants<Constant>(
		pipelineLayout->getHandle(),
		vk::ShaderStageFlagBits::eVertex, 0,
		{ Constant{
			camera->getPVPrev(), 
			camera->getPV(), 
			camera->getPrevJitter(), camera->getCurrJitter()} });
	
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline->getHandle());

	commandBuffer.setViewport(0, 1, &viewport);
	commandBuffer.setScissor(0, 1, &scissor);

	commandBuffer.bindVertexBuffers(0, vertexBuffer->getHandle(), { 0 });
	commandBuffer.bindIndexBuffer(indexBuffer->getHandle(), 0, vk::IndexType::eUint32);

	for (int i = 0; i < scene->getMeshCount(); i++) {
		auto uniformOffset = scene->bufferOffsets[i];
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout->getHandle(), 0, { descriptorSet->getHandle() }, { uniformOffset });
		auto count = scene->getMeshes()[i]->getIndices().size();
		auto vertexOffset = scene->vertexOffsets[i];
		auto indexOffset = scene->indexOffsets[i];
		commandBuffer.drawIndexed(count, 1, indexOffset, vertexOffset, 0);
	}

	commandBuffer.endRendering();
}

void GBufferPass::update(uint32_t frameIndex)
{
	uniforms.clear();
	auto models = scene->getTransforms();
	auto prevModels = scene->getPrevTransforms();
	uniforms.reserve(scene->getMeshCount());
	for (int i = 0; i < scene->getMeshCount(); i++) {
		uniforms.push_back(Uniform(prevModels[i], models[i]));
	}
	uniformBuffer->copyToGPU(static_cast<const void*>(uniforms.data()), sizeof(Uniform) * uniforms.size());
}
