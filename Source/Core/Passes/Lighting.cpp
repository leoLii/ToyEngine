#include "Lighting.hpp"

LightingPass::LightingPass(const GPUContext* context, ResourceManager* resourceManager, const Scene* scene, Vec2 size)
	:gpuContext{ context }
	, resourceManager{ resourceManager }
	, scene{ scene }
{
	width = size.x;
	height = size.y;
	initAttachments();
}

LightingPass::~LightingPass()
{
	delete graphicsPipeline;
	delete pipelineLayout;
	delete descriptorSetLayout;
	delete descriptorSet;
}

void LightingPass::initAttachments()
{
	positionAttachment = resourceManager->getAttachment("gPosition");
	albedoAttachment = resourceManager->getAttachment("gAlbedo");
	normalAttachment = resourceManager->getAttachment("gNormal");
	armAttachment = resourceManager->getAttachment("gARM");

	{
		lightingAttachment = resourceManager->getAttachment("ColorBuffer");
		vk::RenderingAttachmentInfo attachmentInfo{};
		attachmentInfo.imageView = lightingAttachment->view->getHandle();
		attachmentInfo.imageLayout = lightingAttachment->attachmentInfo.layout;
		attachmentInfo.loadOp = lightingAttachment->attachmentInfo.loadOp;
		attachmentInfo.storeOp = lightingAttachment->attachmentInfo.storeOp;
		attachmentInfo.clearValue = lightingAttachment->attachmentInfo.clearValue;
		renderingAttachments.push_back(attachmentInfo);
		attachmentFormats.push_back(lightingAttachment->attachmentInfo.format);
	}

	auto commandBuffer = gpuContext->requestCommandBuffer(CommandType::Transfer, vk::CommandBufferLevel::ePrimary);
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	commandBuffer.begin(beginInfo);

	gpuContext->pipelineBarrier(
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
	renderingInfo.layerCount = 1;
	renderingInfo.renderArea.offset = vk::Offset2D{};
	renderingInfo.renderArea.extent = vk::Extent2D{ width, height };
	renderingInfo.colorAttachmentCount = renderingAttachments.size();
	renderingInfo.pColorAttachments = renderingAttachments.data();

	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = width;
	viewport.height = height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor.offset = vk::Offset2D{ 0, 0 };
	scissor.extent = vk::Extent2D{ width, height };

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
		3, 2, 1
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
	state.dynamicStates.push_back(vk::DynamicState::eViewport);
	state.dynamicStates.push_back(vk::DynamicState::eScissor);
	state.renderingInfo.colorAttachmentFormats = attachmentFormats;

	graphicsPipeline = new GraphicsPipeline(*gpuContext->getDevice(), pipelineLayout, &state, baseModules);

	uniformBuffer = resourceManager->createBuffer(sizeof(Uniform), vk::BufferUsageFlagBits::eUniformBuffer);
	vk::DescriptorBufferInfo descriptorBufferInfo;
	descriptorBufferInfo.buffer = uniformBuffer->getHandle();
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = sizeof(Uniform);

	std::unordered_map<uint32_t, vk::DescriptorBufferInfo> bufferInfos = { {4, descriptorBufferInfo} };

	sampler = resourceManager->createSampler();
	std::unordered_map<uint32_t, vk::DescriptorImageInfo> imageInfos;
	imageInfos[0] = vk::DescriptorImageInfo{ sampler, positionAttachment->view->getHandle(), positionAttachment->attachmentInfo.layout };
	imageInfos[1] = vk::DescriptorImageInfo{ sampler, albedoAttachment->view->getHandle(), albedoAttachment->attachmentInfo.layout };
	imageInfos[2] = vk::DescriptorImageInfo{ sampler, normalAttachment->view->getHandle(), normalAttachment->attachmentInfo.layout };
	imageInfos[3] = vk::DescriptorImageInfo{ sampler, armAttachment->view->getHandle(), armAttachment->attachmentInfo.layout };

	descriptorSet = gpuContext->requireDescriptorSet(descriptorSetLayout, bufferInfos, imageInfos);

	vertexBuffer = resourceManager->createBuffer(vertices.size() * sizeof(float), vk::BufferUsageFlagBits::eVertexBuffer);
	vertexBuffer->copyToGPU(static_cast<const void*>(vertices.data()), vertices.size() * sizeof(float));

	indexBuffer = resourceManager->createBuffer(indices.size() * sizeof(uint32_t), vk::BufferUsageFlagBits::eIndexBuffer);
	indexBuffer->copyToGPU(static_cast<const void*>(indices.data()), indices.size() * sizeof(uint32_t));
}

void LightingPass::record(vk::CommandBuffer commandBuffer)
{
	auto camera = scene->getCamera();

	gpuContext->pipelineBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eFragmentShader,
		vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eShaderRead,
		vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
		positionAttachment->image);

	gpuContext->pipelineBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eFragmentShader,
		vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eShaderRead,
		vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
		albedoAttachment->image);

	gpuContext->pipelineBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eFragmentShader,
		vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eShaderRead,
		vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
		normalAttachment->image);

	gpuContext->pipelineBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eFragmentShader,
		vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eShaderRead,
		vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
		armAttachment->image);

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
		-1.0, 0.0, -1.0
	);

	uniformBuffer->copyToGPU(static_cast<const void*>(&uniform), sizeof(Uniform));
}
