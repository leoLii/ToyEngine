#include "Application.hpp"

#include "Scene/Scene.hpp"
#include "Scene/Components/Camera.hpp"

#include <cstddef>

//void recreateSwapChain()
//{
//    int width = 0, height = 0;
//    window->getFramebufferSize(&width, &height);
//    while (width == 0 || height == 0)
//    {
//        window->getFramebufferSize(&width, &height);
//        window->waitEvents();
//    }
//
//    gpuContext->getDevice()->getHandle().waitIdle();
//
//    framebuffers.clear();
//    swapChainImageViews.clear();
//
//    gpuContext->rebuildSwapchainWithSize(vk::Extent2D(width, height));
//    auto spImages = gpuContext->getSwapchainImages();
//    swapChainImages.resize(spImages.size());
//
//    std::transform(spImages.begin(), spImages.end(), swapChainImages.begin(), [](vk::Image i)->VkImage {
//        return static_cast<VkImage>(i);
//        });
//    swapChainImageFormat = static_cast<VkFormat>(gpuContext->getSwapchainFormat());
//    swapChainExtent = static_cast<VkExtent2D>(gpuContext->getSwapchainExtent());
//    //swapchain->rebuildWithSize(vk::Extent2D(width, height));
//    for (int i = 0; i < swapChainImages.size(); i++) {
//        swapChainImageViews.push_back(gpuContext->createImageView(swapChainImages[i]));
//    }
//
//    for (int i = 0; i < swapChainImageViews.size(); i++) {
//        std::vector<vk::ImageView> temp = { swapChainImageViews[i]->getHandle() };
//        auto f = new Framebuffer{ *gpuContext->getDevice(), *renderPass, temp };
//        framebuffers.push_back(f);
//    }
//}

void Application::init(ApplicationConfig& config, Scene* scene)
{
    window = std::make_unique<Window>(config.name, config.width, config.height);
    auto windowExtensions = Window::requireWindowExtensions();
    config.extensions.insert(config.extensions.end(), windowExtensions.begin(), windowExtensions.end());

    gpuContext = std::make_unique<GPUContext>(config.name, config.layers, config.extensions, window.get());

    auto vertShaderModule = gpuContext->findShader("base.vert");
    auto fragShaderModule = gpuContext->findShader("base.frag");

    std::vector<ShaderModule*> modules = { vertShaderModule.get(), fragShaderModule.get()};

    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    bindings.push_back(vk::DescriptorSetLayoutBinding{ 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex });
    descriptorSetLayout = new DescriptorSetLayout{ *gpuContext->getDevice(), 0, bindings };
    std::vector<vk::DescriptorSetLayout> setLayouts;
    setLayouts.push_back(descriptorSetLayout->getHandle());
    std::vector<vk::PushConstantRange> pushConstanceRanges;

    descriptorSets = gpuContext->requireDescriptorSet(setLayouts);

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
    state.renderingInfo.colorAttachmentFormats.push_back(gpuContext->getSwapchainFormat());
    
    pipelineLayout = new PipelineLayout{ *gpuContext->getDevice(), setLayouts, pushConstanceRanges };
    graphicsPipeline = new GraphicsPipeline(*gpuContext->getDevice(), *pipelineLayout, state, modules);

    fence = gpuContext->requestFence();

    imageAvailableSemaphore = gpuContext->requestSemaphore();

    renderFinishedSemaphore = gpuContext->requestSemaphore();

    this->scene = scene;

    vertices = scene->getMeshes()[0]->assembleVertexData();
    vertexBuffer = gpuContext->createBuffer(vertices.size() * sizeof(Vertex), vk::BufferUsageFlagBits::eVertexBuffer);
    vertexBuffer->copyToGPU(static_cast<const void*>(vertices.data()), vertices.size() * sizeof(Vertex));
    
    indices = scene->getMeshes()[0]->getIndices();
    indexBuffer = gpuContext->createBuffer(indices.size() * sizeof(uint32_t), vk::BufferUsageFlagBits::eIndexBuffer);
    indexBuffer->copyToGPU(static_cast<const void*>(indices.data()), indices.size() * sizeof(uint32_t));

    auto camera = scene->getCamera();
    camera->lookAt(Vec3(0.0f, 0.0f, 10.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f));
    auto model = Mat4(1.0);
    auto clip = Mat4{
        1.0f,  0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f,  0.0f, 0.5f, 0.0f,
        0.0f,  0.0f, 0.5f, 1.0f
    };
    auto view = camera->getViewMatrix();
    auto projection = camera->getProjectionMatrix();
    auto mvp = clip * projection * view * model;
    uniformBuffer = gpuContext->createBuffer(sizeof(Mat4), vk::BufferUsageFlagBits::eUniformBuffer);
    uniformBuffer->copyToGPU(static_cast<const void*>(glm::value_ptr(mvp)), sizeof(mvp));

    vk::DescriptorBufferInfo descriptorBufferInfo;
    descriptorBufferInfo.buffer = uniformBuffer->getHandle();
    descriptorBufferInfo.offset = 0;
    descriptorBufferInfo.range = sizeof(Mat4);

    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.dstSet = descriptorSets[0];
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eUniformBuffer;
    writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;

    gpuContext->getDevice()->getHandle().updateDescriptorSets(writeDescriptorSet, nullptr);
    commandBuffer = gpuContext->requestCommandBuffer(vk::CommandBufferLevel::ePrimary);
}

void Application::beginFrame()
{
    commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    commandBuffer.begin(beginInfo);
}

void Application::endFrame(uint32_t index)
{
    vk::ImageMemoryBarrier imageMemoryBarrier;
    imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
    imageMemoryBarrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
    imageMemoryBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
    imageMemoryBarrier.image = gpuContext->getSwapchainImages()[index]->getHandle();
    imageMemoryBarrier.subresourceRange = vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
    imageMemoryBarrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    imageMemoryBarrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;

    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eBottomOfPipe,
        vk::DependencyFlagBits::eByRegion,
        0, 0, { imageMemoryBarrier });

    commandBuffer.end();

    vk::SubmitInfo submitInfo;

    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

    gpuContext->getDevice()->getGraphicsQueue().submit(submitInfo, fence);
    gpuContext->waitForFences(fence);
}

void Application::present(uint32_t index)
{
    vk::PresentInfoKHR presentInfo;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphore;

    auto swapChain = gpuContext->getSwapchain()->getHandle();
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain;
    presentInfo.pImageIndices = &index;

    auto result = gpuContext->getDevice()->getPresentQueue().presentKHR(presentInfo);

    /*if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized)
    {
        framebufferResized = false;
        recreateSwapChain();
    }*/
}

void Application::recordCommandBuffer(uint32_t index) 
{
    vk::ImageMemoryBarrier imageMemoryBarrier;
    imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
    imageMemoryBarrier.oldLayout = vk::ImageLayout::eUndefined;
    imageMemoryBarrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
    imageMemoryBarrier.image = gpuContext->getSwapchainImages()[index]->getHandle();
    imageMemoryBarrier.subresourceRange = vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
    imageMemoryBarrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    imageMemoryBarrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    
    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::DependencyFlagBits::eByRegion,
        0, 0, { imageMemoryBarrier });

    vk::RenderingAttachmentInfo colorAttachment;
    colorAttachment.imageView = gpuContext->getSwapchainImageViews()[index]->getHandle();  // 使用交换链图像视图
    colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;  // 图像布局
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;  // 清除操作
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;  // 存储操作
    colorAttachment.clearValue.color = vk::ClearColorValue{ 0.0f, 0.0f, 0.0f, 1.0f };// 清除颜色值（黑色）

    vk::RenderingInfo renderingInfo;
    renderingInfo.layerCount = 1;
    renderingInfo.renderArea.offset = vk::Offset2D{};
    renderingInfo.renderArea.extent = gpuContext->getSwapchainExtent();
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

    commandBuffer.beginRendering(&renderingInfo);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout->getHandle(), 0, descriptorSets, nullptr);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline->getHandle());
    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)1920;
    viewport.height = (float)1080;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{ 0, 0 };
    scissor.extent = gpuContext->getSwapchainExtent();
    commandBuffer.setScissor(0, 1, &scissor);

    commandBuffer.bindVertexBuffers(0, vertexBuffer->getHandle(), { 0 });
    commandBuffer.bindIndexBuffer(indexBuffer->getHandle(), 0, vk::IndexType::eUint32);
    commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);

    commandBuffer.endRendering();
}

void Application::run()
{
    while (!window->shouldClose()) {
        ++frameIndex;

        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        deltaTime = std::chrono::duration<double, std::milli>(now - lastFrameTime).count() / 1000.0;
        lastFrameTime = now;

        window->pollEvents();

        gpuContext->waitForFences(fence);

        auto acquieResult =
            gpuContext->acquireNextImage(imageAvailableSemaphore, VK_NULL_HANDLE);

        uint32_t swapChainIndex = std::get<1>(acquieResult);

        /*if (std::get<0>(acquieResult) == vk::Result::eErrorOutOfDateKHR)
        {
            recreateSwapChain();
            return;
        }
        else if (std::get<0>(acquieResult) != vk::Result::eSuccess && std::get<0>(acquieResult) != vk::Result::eSuboptimalKHR) { throw std::runtime_error("failed to acquire swap chain image!"); }*/

        gpuContext->resetFences(fence);

        beginFrame();

        recordCommandBuffer(swapChainIndex);

        endFrame(swapChainIndex);

        present(swapChainIndex);
    }
}

void Application::close()
{
    gpuContext->getDevice()->getHandle().waitIdle();
    
    gpuContext->destroyBuffer(vertexBuffer);
    gpuContext->destroyBuffer(uniformBuffer);
    gpuContext->destroyBuffer(indexBuffer);
    gpuContext->returnSemaphore(renderFinishedSemaphore);
    gpuContext->returnSemaphore(imageAvailableSemaphore);
    gpuContext->returnFence(fence);
    delete descriptorSetLayout;
    delete pipelineLayout;
    delete graphicsPipeline;
}
