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

    for (int i = 0; i < gpuContext->getSwapchainImageCount(); i++) {
        auto swapChainImage = gpuContext->getSwapchainImages()[i];
        std::vector<vk::ImageView> temp =
        { gpuContext->createImageView(swapChainImage.getHandle())->getHandle() };
        auto f = new Framebuffer{ *gpuContext->getDevice(), *renderPass, temp };
        framebuffers.push_back(f);
    }

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

void Application::endFrame()
{
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

void Application::recordCommandBuffer(vk::CommandBuffer commandBuffer, int imageIndex) 
{
    vk::RenderPassBeginInfo renderPassInfo;

    renderPassInfo.renderPass = renderPass->getHandle();
    renderPassInfo.framebuffer = framebuffers[imageIndex]->getHandle();
    renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
    renderPassInfo.renderArea.extent = vk::Extent2D{ 1920, 1080 };


    vk::ClearColorValue color{ 0.0f, 0.0f, 0.0f, 1.0f };
    vk::ClearValue clearValue{ color };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValue;

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
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
    scissor.extent = vk::Extent2D(1920, 1080);
    commandBuffer.setScissor(0, 1, &scissor);

    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRenderPass();

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

        recordCommandBuffer(commandBuffer, swapChainIndex);

        endFrame();

        present(swapChainIndex);
    }
}

void Application::close()
{
    gpuContext->returnSemaphore(renderFinishedSemaphore);
    gpuContext->returnSemaphore(imageAvailableSemaphore);

    gpuContext->returnFence(fence);
    delete graphicsPipeline;
    delete renderPass;
    delete commandPool;
}
