#include "GBuffer.hpp"

GBufferPass::GBufferPass(const GPUContext* context, const Scene* scene)
	:gpuContext{ context }
	, scene{ scene }
{
	positionAttachment = new Attachment{};
	albedoAttachment = new Attachment{};
	normalAttachment = new Attachment{};
	armAttachment = new Attachment{};
	motionAttachment = new Attachment{};
	depthAttachment = new Attachment{};
}

GBufferPass::~GBufferPass()
{
	gpuContext->destroyBuffer(uniformBuffer);
	gpuContext->destroyBuffer(indexBuffer);
	gpuContext->destroyBuffer(vertexBuffer);
	//gpuContext->destroyBuffer(indirectDrawBuffer);
	gpuContext->destroyImage(positionAttachment->image);
	gpuContext->destroyImage(albedoAttachment->image);
	gpuContext->destroyImage(normalAttachment->image);
	gpuContext->destroyImage(armAttachment->image);
	gpuContext->destroyImage(motionAttachment->image);
	gpuContext->destroyImage(depthAttachment->image);
	gpuContext->destroyImageView(positionAttachment->view);
	gpuContext->destroyImageView(albedoAttachment->view);
	gpuContext->destroyImageView(normalAttachment->view);
	gpuContext->destroyImageView(armAttachment->view);
	gpuContext->destroyImageView(motionAttachment->view);
	gpuContext->destroyImageView(depthAttachment->view);
	delete graphicsPipeline;
	delete pipelineLayout;
	delete descriptorSetLayout;
	delete descriptorSet;
	//delete pipelineState;
}

void GBufferPass::initAttachments()
{
	{
		ImageInfo imageInfo{};
		imageInfo.format = vk::Format::eR16G16B16A16Sfloat;
		imageInfo.type = vk::ImageType::e2D;
		imageInfo.extent = vk::Extent3D{ width, height, 1 };
		imageInfo.usage =
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eSampled;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.arrayLayers = 1;
		imageInfo.mipmapLevel = 1;
		imageInfo.queueFamilyCount = 1;
		imageInfo.pQueueFamilyIndices = { 0 };

		positionAttachment->image = gpuContext->createImage(imageInfo);
		positionAttachment->view = gpuContext->createImageView(positionAttachment->image);
		positionAttachment->format = imageInfo.format;
		positionAttachment->attachmentInfo.imageView = positionAttachment->view->getHandle();
		positionAttachment->attachmentInfo.imageLayout = vk::ImageLayout::eGeneral;
		positionAttachment->attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
		positionAttachment->attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
		positionAttachment->attachmentInfo.clearValue.color = vk::ClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f };
		positionAttachment->attachmentInfo.clearValue.depthStencil = vk::ClearDepthStencilValue{ 0u, 0u };

		renderingAttachments.push_back(positionAttachment->attachmentInfo);
		attachmentFormats.push_back(imageInfo.format);
	}

	{
		ImageInfo imageInfo{};
		imageInfo.format = vk::Format::eR8G8B8A8Unorm;
		imageInfo.type = vk::ImageType::e2D;
		imageInfo.extent = vk::Extent3D{ width, height, 1 };
		imageInfo.usage =
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eSampled |
			vk::ImageUsageFlagBits::eTransferSrc;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.arrayLayers = 1;
		imageInfo.mipmapLevel = 1;
		imageInfo.queueFamilyCount = 1;
		imageInfo.pQueueFamilyIndices = { 0 };

		albedoAttachment->image = gpuContext->createImage(imageInfo);
		albedoAttachment->view = gpuContext->createImageView(albedoAttachment->image);
		albedoAttachment->format = imageInfo.format;
		albedoAttachment->attachmentInfo.imageView = albedoAttachment->view->getHandle();
		albedoAttachment->attachmentInfo.imageLayout = vk::ImageLayout::eGeneral;
		albedoAttachment->attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
		albedoAttachment->attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
		albedoAttachment->attachmentInfo.clearValue.color = vk::ClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f };
		albedoAttachment->attachmentInfo.clearValue.depthStencil = vk::ClearDepthStencilValue{ 0u, 0u };
	
		renderingAttachments.push_back(albedoAttachment->attachmentInfo);
		attachmentFormats.push_back(imageInfo.format);
	}

	{
		ImageInfo imageInfo{};
		imageInfo.format = vk::Format::eR16G16B16A16Sfloat;
		imageInfo.type = vk::ImageType::e2D;
		imageInfo.extent = vk::Extent3D{ width, height, 1 };
		imageInfo.usage =
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eSampled;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.arrayLayers = 1;
		imageInfo.mipmapLevel = 1;
		imageInfo.queueFamilyCount = 1;
		imageInfo.pQueueFamilyIndices = { 0 };

		normalAttachment->image = gpuContext->createImage(imageInfo);
		normalAttachment->view = gpuContext->createImageView(normalAttachment->image);
		normalAttachment->format = imageInfo.format;
		normalAttachment->attachmentInfo.imageView = normalAttachment->view->getHandle();
		normalAttachment->attachmentInfo.imageLayout = vk::ImageLayout::eGeneral;
		normalAttachment->attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
		normalAttachment->attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
		normalAttachment->attachmentInfo.clearValue.color = vk::ClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f };
		normalAttachment->attachmentInfo.clearValue.depthStencil = vk::ClearDepthStencilValue{ 0u, 0u };
	
		renderingAttachments.push_back(normalAttachment->attachmentInfo);
		attachmentFormats.push_back(imageInfo.format);
	}
	
	{
		ImageInfo imageInfo{};
		imageInfo.format = vk::Format::eR8G8B8A8Unorm;
		imageInfo.type = vk::ImageType::e2D;
		imageInfo.extent = vk::Extent3D{ width, height, 1 };
		imageInfo.usage =
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eSampled;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.arrayLayers = 1;
		imageInfo.mipmapLevel = 1;
		imageInfo.queueFamilyCount = 1;
		imageInfo.pQueueFamilyIndices = { 0 };

		armAttachment->image = gpuContext->createImage(imageInfo);
		armAttachment->view = gpuContext->createImageView(armAttachment->image);
		armAttachment->format = imageInfo.format;
		armAttachment->attachmentInfo.imageView = armAttachment->view->getHandle();
		armAttachment->attachmentInfo.imageLayout = vk::ImageLayout::eGeneral;
		armAttachment->attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
		armAttachment->attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
		armAttachment->attachmentInfo.clearValue.color = vk::ClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f };
		armAttachment->attachmentInfo.clearValue.depthStencil = vk::ClearDepthStencilValue{ 0u, 0u };
	
		renderingAttachments.push_back(armAttachment->attachmentInfo);
		attachmentFormats.push_back(imageInfo.format);
	}

	{
		ImageInfo imageInfo{};
		imageInfo.format = vk::Format::eR16G16Sfloat;
		imageInfo.type = vk::ImageType::e2D;
		imageInfo.extent = vk::Extent3D{ width, height, 1 };
		imageInfo.usage =
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eSampled;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.arrayLayers = 1;
		imageInfo.mipmapLevel = 1;
		imageInfo.queueFamilyCount = 1;
		imageInfo.pQueueFamilyIndices = { 0 };

		motionAttachment->image = gpuContext->createImage(imageInfo);
		motionAttachment->view = gpuContext->createImageView(motionAttachment->image);
		motionAttachment->format = imageInfo.format;
		motionAttachment->attachmentInfo.imageView = motionAttachment->view->getHandle();
		motionAttachment->attachmentInfo.imageLayout = vk::ImageLayout::eGeneral;
		motionAttachment->attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
		motionAttachment->attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
		motionAttachment->attachmentInfo.clearValue.color = vk::ClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f };
		motionAttachment->attachmentInfo.clearValue.depthStencil = vk::ClearDepthStencilValue{ 0u, 0u };
	
		renderingAttachments.push_back(motionAttachment->attachmentInfo);
		attachmentFormats.push_back(imageInfo.format);
	}
	
	{
		ImageInfo imageInfo{};
		imageInfo.format = vk::Format::eD32Sfloat;
		imageInfo.type = vk::ImageType::e2D;
		imageInfo.extent = vk::Extent3D{ width, height, 1 };
		imageInfo.usage =
			vk::ImageUsageFlagBits::eDepthStencilAttachment |
			vk::ImageUsageFlagBits::eSampled;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.arrayLayers = 1;
		imageInfo.mipmapLevel = 1;
		imageInfo.queueFamilyCount = 1;
		imageInfo.pQueueFamilyIndices = { 0 };

		depthAttachment->image = gpuContext->createImage(imageInfo);
		depthAttachment->view = gpuContext->createImageView(
			depthAttachment->image, vk::ImageViewType::e2D,
			vk::ComponentMapping{}, 
			vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1});
		depthAttachment->format = imageInfo.format;
		depthAttachment->attachmentInfo.imageView = depthAttachment->view->getHandle();
		depthAttachment->attachmentInfo.imageLayout = vk::ImageLayout::eGeneral;
		depthAttachment->attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
		depthAttachment->attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
		depthAttachment->attachmentInfo.clearValue.color = vk::ClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f };
		depthAttachment->attachmentInfo.clearValue.depthStencil = vk::ClearDepthStencilValue{ 0u, 0u };
	}

	auto commandBuffer = gpuContext->requestCommandBuffer(CommandType::Transfer, vk::CommandBufferLevel::ePrimary);
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	commandBuffer.begin(beginInfo);

	gpuContext->transferImage(
		commandBuffer,
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
		positionAttachment->image);

	gpuContext->transferImage(
		commandBuffer,
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
		albedoAttachment->image);

	gpuContext->transferImage(
		commandBuffer,
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
		normalAttachment->image);

	gpuContext->transferImage(
		commandBuffer,
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
		armAttachment->image);

	gpuContext->transferImage(
		commandBuffer,
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
		motionAttachment->image);

	gpuContext->transferImage(
		commandBuffer,
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eAllGraphics,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eDepthStencilAttachmentWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
		depthAttachment->image, vk::DependencyFlagBits::eByRegion,
		vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });

	commandBuffer.end();

	gpuContext->submit(CommandType::Transfer, {}, {}, { commandBuffer }, {}, VK_NULL_HANDLE);
	
	gpuContext->getDevice()->getTransferQueue().waitIdle();
}

void GBufferPass::prepare()
{
	initAttachments();

	renderingInfo.layerCount = 1;
	renderingInfo.renderArea.offset = vk::Offset2D{};
	renderingInfo.renderArea.extent = vk::Extent2D{ 960, 540 };
	renderingInfo.colorAttachmentCount = renderingAttachments.size();
	renderingInfo.pColorAttachments = renderingAttachments.data();
	renderingInfo.pDepthAttachment = &depthAttachment->attachmentInfo;

	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = gpuContext->getSwapchainExtent().width / 2;
	viewport.height = gpuContext->getSwapchainExtent().height / 2;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor.offset = vk::Offset2D{ 0, 0 };
	scissor.extent = gpuContext->getSwapchainExtent();

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

	uniformBuffer = gpuContext->createBuffer(sizeof(Uniform) * scene->getMeshCount(), vk::BufferUsageFlagBits::eUniformBuffer);
	
	vk::DescriptorBufferInfo descriptorBufferInfo;
	descriptorBufferInfo.buffer = uniformBuffer->getHandle();
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = sizeof(Uniform);

	std::unordered_map<uint32_t, vk::DescriptorBufferInfo> bufferInfos = { {0, descriptorBufferInfo} };
	std::unordered_map<uint32_t, vk::DescriptorImageInfo> imageInfos;
	descriptorSet = gpuContext->requireDescriptorSet(descriptorSetLayout, bufferInfos, imageInfos);
	descriptorSet->updateDescriptorSet(0);

	auto vertices = scene->getVertices();
	vertexBuffer = gpuContext->createBuffer(vertices.size() * sizeof(Vertex), vk::BufferUsageFlagBits::eVertexBuffer);
	vertexBuffer->copyToGPU(static_cast<const void*>(vertices.data()), vertices.size() * sizeof(Vertex));

	auto indices = scene->getIndices();
	indexBuffer = gpuContext->createBuffer(indices.size() * sizeof(uint32_t), vk::BufferUsageFlagBits::eIndexBuffer);
	indexBuffer->copyToGPU(static_cast<const void*>(indices.data()), indices.size() * sizeof(uint32_t));
}

void GBufferPass::record(vk::CommandBuffer commandBuffer)
{
	auto camera = scene->getCamera();

	commandBuffer.beginRendering(&renderingInfo);

	commandBuffer.pushConstants<Constant>(
		pipelineLayout->getHandle(), 
		vk::ShaderStageFlagBits::eVertex, 0,
		{ Constant(camera->getPVPrev(), camera->getPVJittered()) });
	
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

void GBufferPass::update()
{
	uniforms.clear();

	auto models = scene->getUniforms();
	auto prevModels = scene->getPrevUniforms();
	uniforms.reserve(scene->getMeshCount());
	for (int i = 0; i < scene->getMeshCount(); i++) {
		uniforms.push_back(Uniform(prevModels[i], models[i]));
	}
	uniformBuffer->copyToGPU(static_cast<const void*>(uniforms.data()), sizeof(Uniform) * uniforms.size());
}
