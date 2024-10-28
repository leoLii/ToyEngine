#include "Lighting.hpp"

LightingPass::LightingPass(const GPUContext* context, const Scene* scene)
	:gpuContext{ context }
	, scene{ scene }
{
	positionAttachment = new Attachment{};
	albedoAttachment = new Attachment{};
	normalAttachment = new Attachment{};
	armAttachment = new Attachment{};
	lightingAttachment = new Attachment{};
}

LightingPass::~LightingPass()
{
	gpuContext->destroyBuffer(uniformBuffer);
	gpuContext->destroyBuffer(vertexBuffer);
	gpuContext->destroyBuffer(indexBuffer);
	gpuContext->destroySampler(sampler);
	gpuContext->destroyImage(lightingAttachment->image);
	gpuContext->destroyImageView(lightingAttachment->view);
	delete graphicsPipeline;
	delete pipelineLayout;
	delete descriptorSetLayout;
	delete descriptorSet;
}

void LightingPass::initAttachments()
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

	lightingAttachment->image = gpuContext->createImage(imageInfo);
	lightingAttachment->view = gpuContext->createImageView(lightingAttachment->image);
	lightingAttachment->format = imageInfo.format;
	lightingAttachment->attachmentInfo.imageView = lightingAttachment->view->getHandle();
	lightingAttachment->attachmentInfo.imageLayout = vk::ImageLayout::eGeneral;
	lightingAttachment->attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
	lightingAttachment->attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
	lightingAttachment->attachmentInfo.clearValue.color = vk::ClearColorValue{ 1.0f, 0.0f, 0.0f, 0.0f };
	lightingAttachment->attachmentInfo.clearValue.depthStencil = vk::ClearDepthStencilValue{ 0u, 0u };

	renderingAttachments.push_back(lightingAttachment->attachmentInfo);
	attachmentFormats.push_back(lightingAttachment->format);

	auto commandBuffer = gpuContext->requestCommandBuffer(CommandType::Transfer, vk::CommandBufferLevel::ePrimary);
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	commandBuffer.begin(beginInfo);

	gpuContext->transferImage(
		commandBuffer,
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
		lightingAttachment->image);
	commandBuffer.end();

	gpuContext->submit(CommandType::Transfer, {}, {}, { commandBuffer }, {}, VK_NULL_HANDLE);

	gpuContext->getDevice()->getTransferQueue().waitIdle();
}

void LightingPass::prepare()
{
	initAttachments();

	renderingInfo.layerCount = 1;
	renderingInfo.renderArea.offset = vk::Offset2D{};
	renderingInfo.renderArea.extent = vk::Extent2D{ 960, 540 };
	renderingInfo.colorAttachmentCount = renderingAttachments.size();
	renderingInfo.pColorAttachments = renderingAttachments.data();

	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = gpuContext->getSwapchainExtent().width / 2;
	viewport.height = gpuContext->getSwapchainExtent().height / 2;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor.offset = vk::Offset2D{ 0, 0 };
	scissor.extent = vk::Extent2D{ 960, 540 };

	std::vector<const ShaderModule*> baseModules = { gpuContext->findShader("deferredlighting.vert"), gpuContext->findShader("deferredlighting.frag") };
	std::vector<vk::DescriptorSetLayoutBinding> bindings;
	bindings.push_back(vk::DescriptorSetLayoutBinding{ 0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment });
	bindings.push_back(vk::DescriptorSetLayoutBinding{ 1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment });
	bindings.push_back(vk::DescriptorSetLayoutBinding{ 2, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment });
	bindings.push_back(vk::DescriptorSetLayoutBinding{ 3, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment });
	bindings.push_back(vk::DescriptorSetLayoutBinding{ 4, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment });
	descriptorSetLayout = gpuContext->createDescriptorSetLayout(0, bindings);

	constants.stageFlags = vk::ShaderStageFlagBits::eFragment;
	constants.size = sizeof(Constant);
	constants.offset = 0;

	pipelineLayout = new PipelineLayout{ *gpuContext->getDevice(), { descriptorSetLayout->getHandle() }, {constants} };

	vertices = {
		// 顶点坐标       // 纹理坐标
		-1.0f, -1.0f,     0.0f, 1.0f,  // 左下角
		 1.0f, -1.0f,     1.0f, 1.0f,  // 右下角
		-1.0f,  1.0f,     0.0f, 0.0f,  // 左上角
		 1.0f,  1.0f,     1.0f, 0.0f   // 右上角
	};

	indices = {
		0, 1, 2,
		1, 3, 2
	};

	std::vector<vk::VertexInputBindingDescription> vertexBindings;
	vertexBindings.push_back(
		vk::VertexInputBindingDescription{ 0, 4 * sizeof(float), vk::VertexInputRate::eVertex });

	std::vector<vk::VertexInputAttributeDescription> vertexAttributes;
	// Position
	vertexAttributes.push_back(
		vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, 0));
	// Texcoord
	vertexAttributes.push_back(
		vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, 2 * sizeof(float)));

	GraphicsPipelineState state;
	state.vertexInputState.bindings = vertexBindings;
	state.vertexInputState.attributes = vertexAttributes;
	state.inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleStrip;
	state.dynamicStates.push_back(vk::DynamicState::eViewport);
	state.dynamicStates.push_back(vk::DynamicState::eScissor);
	state.renderingInfo.colorAttachmentFormats = attachmentFormats;

	graphicsPipeline = new GraphicsPipeline(*gpuContext->getDevice(), pipelineLayout, &state, baseModules);

	uniformBuffer = gpuContext->createBuffer(sizeof(Uniform), vk::BufferUsageFlagBits::eUniformBuffer);
	vk::DescriptorBufferInfo descriptorBufferInfo;
	descriptorBufferInfo.buffer = uniformBuffer->getHandle();
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = sizeof(Uniform);

	std::unordered_map<uint32_t, vk::DescriptorBufferInfo> bufferInfos = { {4, descriptorBufferInfo} };

	sampler = gpuContext->createSampler();
	std::unordered_map<uint32_t, vk::DescriptorImageInfo> imageInfos;
	imageInfos[0] = vk::DescriptorImageInfo{ sampler, positionAttachment->view->getHandle(), positionAttachment->attachmentInfo.imageLayout };
	imageInfos[1] = vk::DescriptorImageInfo{ sampler, albedoAttachment->view->getHandle(), albedoAttachment->attachmentInfo.imageLayout };
	imageInfos[2] = vk::DescriptorImageInfo{ sampler, normalAttachment->view->getHandle(), normalAttachment->attachmentInfo.imageLayout };
	imageInfos[3] = vk::DescriptorImageInfo{ sampler, armAttachment->view->getHandle(), armAttachment->attachmentInfo.imageLayout };

	descriptorSet = gpuContext->requireDescriptorSet(descriptorSetLayout, bufferInfos, imageInfos);

	vertexBuffer = gpuContext->createBuffer(vertices.size() * sizeof(float), vk::BufferUsageFlagBits::eVertexBuffer);
	vertexBuffer->copyToGPU(static_cast<const void*>(vertices.data()), vertices.size() * sizeof(float));

	indexBuffer = gpuContext->createBuffer(indices.size() * sizeof(uint32_t), vk::BufferUsageFlagBits::eIndexBuffer);
	indexBuffer->copyToGPU(static_cast<const void*>(indices.data()), indices.size() * sizeof(uint32_t));
}

void LightingPass::record(vk::CommandBuffer commandBuffer)
{
	auto camera = scene->getCamera();

	commandBuffer.beginRendering(&renderingInfo);

	commandBuffer.pushConstants<Constant>(
		pipelineLayout->getHandle(),
		vk::ShaderStageFlagBits::eFragment, 0,
		{ Constant(Vec3(camera->getAttachNode()->getTransform()[3])) });

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline->getHandle());

	commandBuffer.setViewport(0, 1, &viewport);
	commandBuffer.setScissor(0, 1, &scissor);

	commandBuffer.bindVertexBuffers(0, vertexBuffer->getHandle(), { 0 });
	commandBuffer.bindIndexBuffer(indexBuffer->getHandle(), 0, vk::IndexType::eUint32);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout->getHandle(), 0, { descriptorSet->getHandle() }, {});
	commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);

	commandBuffer.endRendering();
}

void LightingPass::update(uint32_t frameIndex)
{
	// 使用较低频率的正弦和余弦函数，使光照颜色缓慢变化
	uniform.lightColor = Vec3(
		1.0, 1.0, 1.0
	);

	// 使用缓慢变化的角度，使光源方向平滑旋转
	float angle = frameIndex * 0.005f; // 控制旋转速度
	uniform.lightDirection = Vec3(
		0.0, 1.0, -1.0
	);

	uniformBuffer->copyToGPU(static_cast<const void*>(&uniform), sizeof(Uniform));
}

void LightingPass::setAttachment(uint32_t index, Attachment* attachment)
{
	switch (index)
	{
	case 0:
		positionAttachment = attachment;
		break;
	case 1:
		albedoAttachment = attachment;
		break;
	case 2:
		normalAttachment = attachment;
		break;
	case 3:
		armAttachment = attachment;
		break;
	default:
		break;
	}
}

Attachment* LightingPass::getAttachment()
{
	return lightingAttachment;
}
