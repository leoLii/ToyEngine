#include "BasePass.hpp"

BasePass::BasePass(const GPUContext* context, const Scene* scene)
	:gpuContext{ context }
	, scene{ scene }
{
	colorAttachment = new Attachment{};
	normalAttachment = new Attachment{};
	armAttachment = new Attachment{};
	motionAttachment = new Attachment{};
	depthAttachment = new Attachment{};
}

BasePass::~BasePass()
{
	gpuContext->destroyBuffer(uniformBuffer);
	gpuContext->destroyBuffer(indexBuffer);
	gpuContext->destroyBuffer(vertexBuffer);
	//gpuContext->destroyBuffer(indirectDrawBuffer);
	gpuContext->destroyImage(colorAttachment->image);
	gpuContext->destroyImage(normalAttachment->image);
	gpuContext->destroyImage(armAttachment->image);
	gpuContext->destroyImage(motionAttachment->image);
	gpuContext->destroyImage(depthAttachment->image);
	gpuContext->destroyImageView(colorAttachment->view);
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

void BasePass::initAttachments()
{
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

		colorAttachment->image = gpuContext->createImage(imageInfo);
		colorAttachment->view = gpuContext->createImageView(colorAttachment->image);
		colorAttachment->format = imageInfo.format;
		colorAttachment->attachmentInfo.imageView = colorAttachment->view->getHandle();
		colorAttachment->attachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		colorAttachment->attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachment->attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachment->attachmentInfo.clearValue.color = vk::ClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f };
		colorAttachment->attachmentInfo.clearValue.depthStencil = vk::ClearDepthStencilValue{ 0u, 0u };
	
		renderingAttachments.push_back(colorAttachment->attachmentInfo);
		attachmentFormats.push_back(imageInfo.format);
	}

	{
		ImageInfo imageInfo{};
		imageInfo.format = vk::Format::eR8G8B8A8Snorm;
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
		normalAttachment->attachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
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
		armAttachment->attachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
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
		motionAttachment->attachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
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
		depthAttachment->attachmentInfo.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		depthAttachment->attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
		depthAttachment->attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
		depthAttachment->attachmentInfo.clearValue.color = vk::ClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f };
		depthAttachment->attachmentInfo.clearValue.depthStencil = vk::ClearDepthStencilValue{ 0u, 0u };
	}
}

void BasePass::prepare(vk::CommandBuffer commandBuffer)
{
	initAttachments();

	renderingInfo.layerCount = 1;
	renderingInfo.renderArea.offset = vk::Offset2D{};
	renderingInfo.renderArea.extent = vk::Extent2D{ 960, 540 };
	renderingInfo.colorAttachmentCount = renderingAttachments.size();
	renderingInfo.pColorAttachments = renderingAttachments.data();
	renderingInfo.pDepthAttachment = &depthAttachment->attachmentInfo;

	gpuContext->transferImage(
		commandBuffer,
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
		colorAttachment->image);

	gpuContext->transferImage(
		commandBuffer,
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
		normalAttachment->image);

	gpuContext->transferImage(
		commandBuffer,
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
		armAttachment->image);

	gpuContext->transferImage(
		commandBuffer,
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
		motionAttachment->image);

	gpuContext->transferImage(
		commandBuffer,
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eAllGraphics,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eDepthStencilAttachmentWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal,
		depthAttachment->image, vk::DependencyFlagBits::eByRegion,
		vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });

	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = gpuContext->getSwapchainExtent().width / 2;
	viewport.height = gpuContext->getSwapchainExtent().height / 2;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor.offset = vk::Offset2D{ 0, 0 };
	scissor.extent = gpuContext->getSwapchainExtent();

	std::vector<const ShaderModule*> baseModules = { gpuContext->findShader("base.vert"), gpuContext->findShader("base.frag") };
	vk::DescriptorSetLayoutBinding binding = vk::DescriptorSetLayoutBinding{ 0, vk::DescriptorType::eUniformBufferDynamic, 1, vk::ShaderStageFlagBits::eVertex };
	descriptorSetLayout = gpuContext->createDescriptorSetLayout(0, { binding });

	constants.stageFlags = vk::ShaderStageFlagBits::eVertex;
	constants.size = sizeof(Mat4);
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

	GraphicsPipelineState state;
	state.vertexInputState.bindings = vertexBindings;
	state.vertexInputState.attributes = vertexAttributes;
	state.dynamicStates.push_back(vk::DynamicState::eViewport);
	state.dynamicStates.push_back(vk::DynamicState::eScissor);
	state.renderingInfo.colorAttachmentFormats = attachmentFormats;

	graphicsPipeline = new GraphicsPipeline(*gpuContext->getDevice(), pipelineLayout, &state, baseModules);

	auto models = scene->getUniforms();
	uniformBuffer = gpuContext->createBuffer(sizeof(Mat4) * models.size(), vk::BufferUsageFlagBits::eUniformBuffer);
	
	vk::DescriptorBufferInfo descriptorBufferInfo;
	descriptorBufferInfo.buffer = uniformBuffer->getHandle();
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = sizeof(Mat4);

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

void BasePass::record(vk::CommandBuffer commandBuffer)
{
	auto camera = scene->getCamera();
	auto matrix = camera->getProjectionMatrix() * camera->getViewMatrix();
	commandBuffer.beginRendering(&renderingInfo);

	commandBuffer.pushConstants<Mat4>(pipelineLayout->getHandle(), vk::ShaderStageFlagBits::eVertex, 0, { matrix });
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

void BasePass::update()
{
	auto models = scene->getUniforms();
	uniformBuffer->copyToGPU(static_cast<const void*>(models.data()), sizeof(Mat4) * models.size());
}
