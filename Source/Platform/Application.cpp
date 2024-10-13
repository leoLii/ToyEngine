#include "Application.hpp"


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

void Application::init(ApplicationConfig& config)
{
	window = std::make_unique<Window>(config.name, config.width, config.height);
    auto windowExtensions = Window::requireWindowExtensions();
    config.extensions.insert(config.extensions.end(), windowExtensions.begin(), windowExtensions.end());
	
    gpuContext = std::make_unique<GPUContext>(config.name, config.layers, config.extensions, window.get());

    renderPass = new RenderPass(*gpuContext->getDevice());

    auto vertShaderModule = gpuContext->findShader("triangle.vert");
    auto fragShaderModule = gpuContext->findShader("triangle.frag");

    std::vector<vk::ShaderModule> modules;
    modules.push_back(vertShaderModule->getHandle());
    modules.push_back(fragShaderModule->getHandle());
    graphicsPipeline = new GraphicsPipeline(*gpuContext->getDevice(), *renderPass, modules);

    commandPool = new CommandPool(*gpuContext->getDevice(), 0, config.maxFrames);
    fence = gpuContext->requestFence();

    imageAvailableSemaphore = gpuContext->requestSemaphore();

    renderFinishedSemaphore = gpuContext->requestSemaphore();
}

void Application::beginFrame()
{
    commandBuffer = commandPool->getCommandBuffer(frameIndex % config.maxFrames);
    
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

    commandBuffer.draw(3, 1, 0, 0);

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
    gpuContext->returnSemaphore(renderFinishedSemaphore);
    gpuContext->returnSemaphore(imageAvailableSemaphore);
    gpuContext->returnFence(fence);

    delete graphicsPipeline;
    delete renderPass;
    delete commandPool;
}
