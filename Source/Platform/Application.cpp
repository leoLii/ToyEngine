#include "Application.hpp"

#include "Scene/Scene.hpp"
#include "Scene/Components/Camera.hpp"

#include "Core/Rendering/BasePass.hpp"

#include <cstddef>

void Application::init(ApplicationConfig& config, Scene* scene)
{
	window = std::make_unique<Window>(config.name, config.width, config.height);
	auto windowExtensions = Window::requireWindowExtensions();
	config.extensions.insert(config.extensions.end(), windowExtensions.begin(), windowExtensions.end());

	gpuContext = std::make_unique<GPUContext>(config.name, config.layers, config.extensions, window.get());

	fence = gpuContext->requestFence();

	this->scene = scene;
	basePass = new BasePass{ gpuContext.get(), scene };

	imageAvailableSemaphore = gpuContext->requestSemaphore();
	renderFinishedSemaphore = gpuContext->requestSemaphore();

	basePass->prepare();
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

		commandBuffer = basePass->record();

		std::vector<vk::Semaphore> waitSemaphores{ imageAvailableSemaphore };
		std::vector<vk::PipelineStageFlags> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		std::vector<vk::CommandBuffer> commandBuffers = { commandBuffer };
		std::vector<vk::Semaphore> signalSemaphores = { renderFinishedSemaphore };
		gpuContext->submit(0, waitSemaphores, waitStages, commandBuffers, signalSemaphores, fence);

		/*{
			vk::ImageMemoryBarrier imageMemoryBarrier;
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eNone;
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
			imageMemoryBarrier.oldLayout = vk::ImageLayout::eUndefined;
			imageMemoryBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
			imageMemoryBarrier.image = gpuContext->getSwapchainImages()[swapChainIndex]->getHandle();
			imageMemoryBarrier.subresourceRange = vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
			imageMemoryBarrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
			imageMemoryBarrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;

			commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::DependencyFlagBits::eByRegion,
				0, 0, { imageMemoryBarrier });
		}

		{
			vk::ImageMemoryBarrier imageMemoryBarrier;
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eNone;
			imageMemoryBarrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
			imageMemoryBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
			imageMemoryBarrier.image = gpuContext->getSwapchainImages()[swapChainIndex]->getHandle();
			imageMemoryBarrier.subresourceRange = vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
			imageMemoryBarrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
			imageMemoryBarrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;

			commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::PipelineStageFlagBits::eBottomOfPipe,
				vk::DependencyFlagBits::eByRegion,
				0, 0, { imageMemoryBarrier });
		}*/

		{
			std::vector<vk::Semaphore> waitSemaphores = { renderFinishedSemaphore };
			gpuContext->present(swapChainIndex, waitSemaphores);
		}
	}
}

void Application::close()
{
	gpuContext->getDevice()->getHandle().waitIdle();
	gpuContext->returnSemaphore(renderFinishedSemaphore);
	gpuContext->returnSemaphore(imageAvailableSemaphore);
	gpuContext->returnFence(fence);
}
