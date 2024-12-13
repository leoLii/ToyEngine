//#include "temporalAntiAliasing.hpp"
//
//TaaPass1::TaaPass1(
//	const GPUContext* context, ResourceManager* manager, const Scene* scene, Vec2 size)
//	:GraphicsPass{context, manager, scene, size}
//{
//	initAttachment();
//}
//
//TaaPass1::~TaaPass1()
//{
//}
//
//void TaaPass1::prepare()
//{
//	renderingInfo.layerCount = 1;
//	renderingInfo.renderArea.offset = vk::Offset2D{};
//	renderingInfo.renderArea.extent = vk::Extent2D{ width, height };
//	renderingInfo.colorAttachmentCount = renderingAttachments.size();
//	renderingInfo.pColorAttachments = renderingAttachments.data();
//
//	viewport.x = 0.0f;
//	viewport.y = 0.0f;
//	viewport.width = width;
//	viewport.height = height;
//	viewport.minDepth = 0.0f;
//	viewport.maxDepth = 1.0f;
//
//	scissor.offset = vk::Offset2D{ 0, 0 };
//	scissor.extent = vk::Extent2D{ width, height };
//
//	std::vector<const ShaderModule*> shaderModules = { resourceManager->findShader("taa.vert"), resourceManager->findShader("taa.frag") };
//	std::vector<vk::DescriptorSetLayoutBinding> bindings;
//	bindings.push_back(vk::DescriptorSetLayoutBinding{ 0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment });
//	bindings.push_back(vk::DescriptorSetLayoutBinding{ 1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment });
//	bindings.push_back(vk::DescriptorSetLayoutBinding{ 2, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment });
//	bindings.push_back(vk::DescriptorSetLayoutBinding{ 3, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment });
//	descriptorSetLayout = gpuContext->createDescriptorSetLayout(0, bindings);
//
//	constants.stageFlags = vk::ShaderStageFlagBits::eFragment;
//	constants.size = sizeof(Constant);
//	constants.offset = 0;
//
//	pipelineLayout = new PipelineLayout{ *gpuContext->getDevice(), {descriptorSetLayout->getHandle()}, {constants} };
//	vertices = {
//		// 顶点坐标       // 纹理坐标
//		-1.0f, -1.0f,     0.0f, 0.0f,  // 左下角
//		 1.0f, -1.0f,     1.0f, 0.0f,  // 右下角
//		-1.0f,  1.0f,     0.0f, 1.0f,  // 左上角
//		 1.0f,  1.0f,     1.0f, 1.0f   // 右上角
//	};
//
//	indices = {
//		0, 2, 1,
//		3, 1, 2
//	};
//
//	std::vector<vk::VertexInputBindingDescription> vertexBindings;
//	vertexBindings.push_back(
//		vk::VertexInputBindingDescription{ 0, 4 * sizeof(float), vk::VertexInputRate::eVertex });
//
//	std::vector<vk::VertexInputAttributeDescription> vertexAttributes;
//	// Position
//	vertexAttributes.push_back(
//		vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, 0));
//	// Texcoord
//	vertexAttributes.push_back(
//		vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, 2 * sizeof(float)));
//
//	GraphicsPipelineState state;
//	state.vertexInputState.bindings = vertexBindings;
//	state.vertexInputState.attributes = vertexAttributes;
//	state.dynamicStates.push_back(vk::DynamicState::eViewport);
//	state.dynamicStates.push_back(vk::DynamicState::eScissor);
//	state.renderingInfo.colorAttachmentFormats = attachmentFormats;
//
//	graphicsPipeline = new GraphicsPipeline(*gpuContext->getDevice(), pipelineLayout, VK_NULL_HANDLE, &state, shaderModules);
//
//	sampler1 = resourceManager->createSampler();
//	sampler2 = resourceManager->createSampler(vk::Filter::eNearest, vk::Filter::eNearest);
//	std::unordered_map<uint32_t, vk::DescriptorImageInfo> imageInfos;
//	imageInfos[0] = vk::DescriptorImageInfo{ sampler1, history->view->getHandle(), history->attachmentInfo.layout };
//	imageInfos[1] = vk::DescriptorImageInfo{ sampler1, lightingResult->view->getHandle(), lightingResult->attachmentInfo.layout };
//	imageInfos[2] = vk::DescriptorImageInfo{ sampler2, velocity->view->getHandle(), velocity->attachmentInfo.layout };
//	imageInfos[3] = vk::DescriptorImageInfo{ sampler2, depth->view->getHandle(), depth->attachmentInfo.layout };
//
//	descriptorSet = gpuContext->requireDescriptorSet(descriptorSetLayout, {}, imageInfos);
//	vertexBuffer = resourceManager->createBuffer(vertices.size() * sizeof(float), vk::BufferUsageFlagBits::eVertexBuffer);
//	vertexBuffer->copyToGPU(static_cast<const void*>(vertices.data()), vertices.size() * sizeof(float));
//
//	indexBuffer = resourceManager->createBuffer(indices.size() * sizeof(uint32_t), vk::BufferUsageFlagBits::eIndexBuffer);
//	indexBuffer->copyToGPU(static_cast<const void*>(indices.data()), indices.size() * sizeof(uint32_t));
//
//}
//
//void TaaPass1::update(uint32_t)
//{
//}
//
//void TaaPass1::record(vk::CommandBuffer commandBuffer)
//{
//	gpuContext->pipelineBarrier(
//		commandBuffer,
//		vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
//		vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead,
//		vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
//		history->image);
//
//	gpuContext->pipelineBarrier(
//		commandBuffer,
//		vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eFragmentShader,
//		vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eShaderRead,
//		vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
//		lightingResult->image);
//
//	gpuContext->pipelineBarrier(
//		commandBuffer,
//		vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eFragmentShader,
//		vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eShaderRead,
//		vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
//		velocity->image);
//
//	gpuContext->pipelineBarrier(
//		commandBuffer,
//		vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::PipelineStageFlagBits::eFragmentShader,
//		vk::AccessFlagBits::eDepthStencilAttachmentWrite, vk::AccessFlagBits::eShaderRead,
//		vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
//		depth->image,
//		vk::DependencyFlagBits::eByRegion,
//		vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });
//
//	commandBuffer.beginRendering(&renderingInfo);
//
//	auto camera = scene->getCamera();
//	commandBuffer.pushConstants<Constant>(
//		pipelineLayout->getHandle(),
//		vk::ShaderStageFlagBits::eFragment, 0,
//		{ Constant{Vec2(width, height), camera->getCurrJitter()} });
//
//	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline->getHandle());
//
//	commandBuffer.setViewport(0, 1, &viewport);
//	commandBuffer.setScissor(0, 1, &scissor);
//
//	commandBuffer.bindVertexBuffers(0, vertexBuffer->getHandle(), { 0 });
//	commandBuffer.bindIndexBuffer(indexBuffer->getHandle(), 0, vk::IndexType::eUint32);
//
//	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout->getHandle(), 0, { descriptorSet->getHandle() }, {});
//	commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);
//
//	commandBuffer.endRendering();
//
//	gpuContext->pipelineBarrier(
//		commandBuffer,
//		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eTransfer,
//		vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eTransferRead,
//		vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
//		taaOutput->image);
//
//	gpuContext->pipelineBarrier(
//		commandBuffer,
//		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eTransfer,
//		vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eTransferWrite,
//		vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
//		history->image);
//
//	vk::ImageCopy copyRegion;
//	copyRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
//	copyRegion.srcSubresource.mipLevel = 0;
//	copyRegion.srcSubresource.baseArrayLayer = 0;
//	copyRegion.srcSubresource.layerCount = 1;
//	copyRegion.srcOffset = vk::Offset3D{ 0, 0, 0 };
//
//	copyRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
//	copyRegion.dstSubresource.mipLevel = 0;
//	copyRegion.dstSubresource.baseArrayLayer = 0;
//	copyRegion.dstSubresource.layerCount = 1;
//	copyRegion.dstOffset = vk::Offset3D{ 0, 0, 0 };
//
//	copyRegion.extent.width = width;
//	copyRegion.extent.height = height;
//	copyRegion.extent.depth = 1;
//
//	commandBuffer.copyImage(
//		taaOutput->image->getHandle(), vk::ImageLayout::eGeneral,
//		history->image->getHandle(), vk::ImageLayout::eGeneral, { copyRegion });
//}
//
//void TaaPass1::initAttachment()
//{
//	history = resourceManager->getAttachment("taaHistory");
//	lightingResult = resourceManager->getAttachment("ColorBuffer");
//	velocity = resourceManager->getAttachment("gVelocity");
//	depth = resourceManager->getAttachment("gDepth");
//
//	{
//		taaOutput = resourceManager->getAttachment("taaOutput");
//		vk::RenderingAttachmentInfo attachmentInfo{};
//		attachmentInfo.imageView = taaOutput->view->getHandle();
//		attachmentInfo.imageLayout = taaOutput->attachmentInfo.layout;
//		attachmentInfo.loadOp = taaOutput->attachmentInfo.loadOp;
//		attachmentInfo.storeOp = taaOutput->attachmentInfo.storeOp;
//		attachmentInfo.clearValue = taaOutput->attachmentInfo.clearValue;
//		renderingAttachments.push_back(attachmentInfo);
//		attachmentFormats.push_back(taaOutput->attachmentInfo.format);
//	}
//
//	auto commandBuffer = gpuContext->requestCommandBuffer(CommandType::Transfer, vk::CommandBufferLevel::ePrimary);
//	vk::CommandBufferBeginInfo beginInfo;
//	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
//	commandBuffer.begin(beginInfo);
//
//	gpuContext->pipelineBarrier(
//		commandBuffer,
//		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader,
//		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eShaderRead,
//		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
//		history->image);
//
//	gpuContext->pipelineBarrier(
//		commandBuffer,
//		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader,
//		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eMemoryWrite,
//		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
//		taaOutput->image);
//
//	commandBuffer.end();
//
//	gpuContext->submit(CommandType::Transfer, {}, {}, { commandBuffer }, {}, VK_NULL_HANDLE);
//
//	gpuContext->getDevice()->getTransferQueue().waitIdle();
//}
