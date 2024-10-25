//#include "RenderContext.hpp"
//
//#include "../GPUContext.hpp"
//#include "Common/Math.hpp"
//
//RenderContext::RenderContext(const GPUContext* gpuContext)
//	:gpuContext{ gpuContext }
//{
//}
//
//void RenderContext::basePassInit(
//    vk::CommandBuffer commandBuffer, 
//    std::vector<Vertex>& vertices, 
//    std::vector<uint32_t>& indices,
//    std::vector<Mat4>& models,
//    Mat4 view,
//    Mat4 projection)
//{
//    std::vector<const ShaderModule*> baseModules = { gpuContext->findShader("base.vert"), gpuContext->findShader("base.frag") };
//    std::vector<vk::DescriptorSetLayoutBinding> bindings;
//    bindings.push_back(vk::DescriptorSetLayoutBinding{ 0, vk::DescriptorType::eUniformBufferDynamic, 1, vk::ShaderStageFlagBits::eVertex });
//    descriptorSetLayout = new DescriptorSetLayout{ *gpuContext->getDevice(), 0, bindings };
//    
//    vk::PushConstantRange matrix;
//    matrix.stageFlags = vk::ShaderStageFlagBits::eVertex;
//    matrix.size = sizeof(Mat4);
//    matrix.offset = 0;
//    std::vector<vk::PushConstantRange> pushConstanceRanges = { matrix };
//    std::vector<vk::DescriptorSetLayout>setLayouts = { descriptorSetLayout->getHandle()};
//    basePipelineLayout = new PipelineLayout{ *gpuContext->getDevice(), setLayouts, pushConstanceRanges };
//
//    std::vector<vk::VertexInputBindingDescription> vertexBindings;
//    vertexBindings.push_back(
//        vk::VertexInputBindingDescription{ 0, sizeof(Vertex), vk::VertexInputRate::eVertex });
//
//    std::vector<vk::VertexInputAttributeDescription> vertexAttributes;
//    // Position
//    vertexAttributes.push_back(
//        vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)));
//    // Texcoord
//    vertexAttributes.push_back(
//        vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texcoord)));
//    // Normal
//    vertexAttributes.push_back(
//        vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)));
//
//    GraphicsPipelineState state;
//    state.vertexInputState.bindings = vertexBindings;
//    state.vertexInputState.attributes = vertexAttributes;
//    state.dynamicStates.push_back(vk::DynamicState::eViewport);
//    state.dynamicStates.push_back(vk::DynamicState::eScissor);
//    state.renderingInfo.colorAttachmentFormats.push_back(gpuContext->getSwapchainFormat());
//    
//    basePipeline = gpuContext->createGraphicsPipeline(basePipelineLayout, &state, baseModules);
//
//    basePassUniformBuffer = gpuContext->createBuffer(sizeof(Mat4) * models.size(), vk::BufferUsageFlagBits::eUniformBuffer);
//    basePassUniformBuffer->copyToGPU(static_cast<const void*>(models.data()), sizeof(Mat4) * models.size());
//
//    vk::DescriptorBufferInfo descriptorBufferInfo;
//    descriptorBufferInfo.buffer = basePassUniformBuffer->getHandle();
//    descriptorBufferInfo.offset = 0;
//    descriptorBufferInfo.range = sizeof(Mat4);
//
//    std::unordered_map<uint32_t, vk::DescriptorBufferInfo> bufferInfos;
//    bufferInfos[0] = descriptorBufferInfo;
//    std::unordered_map<uint32_t, vk::DescriptorImageInfo> imageInfos;
//    descriptorSet = gpuContext->requireDescriptorSet(*descriptorSetLayout, bufferInfos, imageInfos);
//    descriptorSet->updateDescriptorSet(0);
//
//    basePassVertexBuffer = gpuContext->createBuffer(vertices.size() * sizeof(Vertex), vk::BufferUsageFlagBits::eVertexBuffer);
//    basePassVertexBuffer->copyToGPU(static_cast<const void*>(vertices.data()), vertices.size() * sizeof(Vertex));
//
//    basePassIndexBuffer = gpuContext->createBuffer(indices.size() * sizeof(uint32_t), vk::BufferUsageFlagBits::eIndexBuffer);
//    basePassIndexBuffer->copyToGPU(static_cast<const void*>(indices.data()), indices.size() * sizeof(uint32_t));
//    size = indices.size();
//    ImageInfo imageInfo{};
//    imageInfo.format = vk::Format::eB8G8R8A8Srgb;
//    imageInfo.type = vk::ImageType::e2D;
//    imageInfo.extent = vk::Extent3D{ 1920, 1080, 1 };
//    imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment;
//    imageInfo.sharingMode = vk::SharingMode::eExclusive;
//    imageInfo.arrayLayers = 1;
//    imageInfo.mipmapLevel = 1;
//    imageInfo.queueFamilyCount = 1;
//    imageInfo.pQueueFamilyIndices = { 0 };
//
//    image = gpuContext->createImage(imageInfo);
//    this->image = image;
//    this->imageView = gpuContext->createImageView(image);
//
//    pvMatrix = projection * view;
//}
//
//void RenderContext::basePassRecord(vk::CommandBuffer commandBuffer)
//{
//    commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
//    vk::CommandBufferBeginInfo beginInfo;
//    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
//    commandBuffer.begin(beginInfo);
//
//    vk::RenderingAttachmentInfo colorAttachment;
//    colorAttachment.imageView = imageView->getHandle();  // 使用交换链图像视图
//    colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;  // 图像布局
//    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;  // 清除操作
//    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;  // 存储操作
//    colorAttachment.clearValue.color = vk::ClearColorValue{ 0.0f, 0.0f, 0.0f, 1.0f };// 清除颜色值（黑色）
//
//    vk::RenderingInfo renderingInfo;
//    renderingInfo.layerCount = 1;
//    renderingInfo.renderArea.offset = vk::Offset2D{};
//    renderingInfo.renderArea.extent = gpuContext->getSwapchainExtent();
//    renderingInfo.colorAttachmentCount = 1;
//    renderingInfo.pColorAttachments = &colorAttachment;
//
//    commandBuffer.beginRendering(&renderingInfo);
//    commandBuffer.pushConstants<Mat4>(basePipelineLayout->getHandle(), vk::ShaderStageFlagBits::eVertex, 0, {pvMatrix});
//    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, basePipelineLayout->getHandle(), 0, { descriptorSet->getHandle() }, { 0 });
//    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, basePipeline->getHandle());
//    vk::Viewport viewport{};
//    viewport.x = 0.0f;
//    viewport.y = 0.0f;
//    viewport.width = (float)1920;
//    viewport.height = (float)1080;
//    viewport.minDepth = 0.0f;
//    viewport.maxDepth = 1.0f;
//
//    commandBuffer.setViewport(0, 1, &viewport);
//
//    vk::Rect2D scissor{};
//    scissor.offset = vk::Offset2D{ 0, 0 };
//    scissor.extent = gpuContext->getSwapchainExtent();
//    commandBuffer.setScissor(0, 1, &scissor);
//
//    commandBuffer.bindVertexBuffers(0, basePassVertexBuffer->getHandle(), { 0 });
//    commandBuffer.bindIndexBuffer(basePassIndexBuffer->getHandle(), 0, vk::IndexType::eUint32);
//    commandBuffer.drawIndexed(size/2, 1, 0, 0, 0);
//
//    commandBuffer.endRendering();
//
//    commandBuffer.end();
//}
//
//void RenderContext::basePassEnd(vk::CommandBuffer commandBuffer)
//{
//}
