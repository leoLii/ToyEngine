#include "BasePass.hpp"

BasePass::BasePass(const GPUContext* context, const Scene* scene)
	:gpuContext{ context }
	, scene{ scene }
{
	colorAttachment = new Attachment{};
	depthAttachment = new Attachment{};
}

BasePass::~BasePass()
{
	gpuContext->destroyBuffer(uniformBuffer);
	gpuContext->destroyBuffer(indexBuffer);
	gpuContext->destroyBuffer(vertexBuffer);
	//gpuContext->destroyBuffer(indirectDrawBuffer);
	gpuContext->destroyImage(colorAttachment->image);
	//gpuContext->destroyImage(depthAttachment->image);
	gpuContext->destroyImageView(colorAttachment->view);
	//gpuContext->destroyImageView(depthAttachment->view);
	delete graphicsPipeline;
	delete pipelineLayout;
	delete descriptorSet;
	delete descriptorSetLayout;
	//delete pipelineState;
}

void BasePass::prepare(vk::CommandBuffer commandBuffer)
{
	{
		ImageInfo imageInfo{};
		imageInfo.format = vk::Format::eB8G8R8A8Srgb;
		imageInfo.type = vk::ImageType::e2D;
		imageInfo.extent = vk::Extent3D{ 1920, 1080, 1 };
		imageInfo.usage =
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eTransferSrc |
			vk::ImageUsageFlagBits::eSampled;
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

		renderingInfo.layerCount = 1;
		renderingInfo.renderArea.offset = vk::Offset2D{};
		renderingInfo.renderArea.extent = gpuContext->getSwapchainExtent();
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment->attachmentInfo;
	}

	gpuContext->transferImage(
		commandBuffer,
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
		colorAttachment->image);

	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = gpuContext->getSwapchainExtent().width;
	viewport.height = gpuContext->getSwapchainExtent().height;
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
	state.renderingInfo.colorAttachmentFormats.push_back(colorAttachment->format);

	graphicsPipeline = new GraphicsPipeline(*gpuContext->getDevice(), pipelineLayout, &state, baseModules);

	auto models = scene->getUniforms();
	uniformBuffer = gpuContext->createBuffer(sizeof(Mat4) * models.size(), vk::BufferUsageFlagBits::eUniformBuffer);
	
	vk::DescriptorBufferInfo descriptorBufferInfo;
	descriptorBufferInfo.buffer = uniformBuffer->getHandle();
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = sizeof(Mat4);

	std::unordered_map<uint32_t, vk::DescriptorBufferInfo> bufferInfos = { {0, descriptorBufferInfo} };
	std::unordered_map<uint32_t, vk::DescriptorImageInfo> imageInfos;
	descriptorSet = gpuContext->requireDescriptorSet(*descriptorSetLayout, bufferInfos, imageInfos);
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
