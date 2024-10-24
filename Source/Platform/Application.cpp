#include "Application.hpp"

#include "Scene/Scene.hpp"
#include "Scene/Components/Camera.hpp"

#include "Core/Rendering/BasePass.hpp"

#include <cstddef>
#include <functional>
#include <thread>
#include <future>

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
	transferFinishedSemaphore = gpuContext->requestSemaphore();

	renderCommandBuffer = gpuContext->requestCommandBuffer(CommandType::Graphics, vk::CommandBufferLevel::ePrimary);
	transferCommandBuffer = gpuContext->requestCommandBuffer(CommandType::Graphics, vk::CommandBufferLevel::ePrimary);

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
	renderCommandBuffer.begin(beginInfo);
	basePass->prepare(renderCommandBuffer);
	basePass->record(renderCommandBuffer);
	renderCommandBuffer.end();
}

void Application::run()
{
	while (!window->shouldClose()) {
		
		++frameIndex;

		scene->update(frameIndex);
		basePass->update();

		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
		deltaTime = std::chrono::duration<double, std::milli>(now - lastFrameTime).count() / 1000.0;
		lastFrameTime = now;

		window->pollEvents();

		auto acquieResult = gpuContext->acquireNextImage(imageAvailableSemaphore, VK_NULL_HANDLE);
		uint32_t swapChainIndex = std::get<1>(acquieResult);

		gpuContext->submit(CommandType::Graphics, {}, {}, { renderCommandBuffer }, { renderFinishedSemaphore }, VK_NULL_HANDLE);
		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		transferCommandBuffer.begin(beginInfo);

		gpuContext->transferImage(
			transferCommandBuffer,
			vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer,
			vk::AccessFlagBits::eNone, vk::AccessFlagBits::eTransferWrite,
			vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
			gpuContext->getSwapchainImages()[swapChainIndex]);

		gpuContext->transferImage(
			transferCommandBuffer,
			vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eTransfer,
			vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eTransferRead,
			vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal,
			basePass->getImage());


		vk::ImageCopy copyRegion;
		copyRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		copyRegion.srcSubresource.mipLevel = 0;
		copyRegion.srcSubresource.layerCount = 1;
		copyRegion.srcOffset = vk::Offset3D{ 0, 0, 0 };
		copyRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		copyRegion.dstSubresource.mipLevel = 0;
		copyRegion.dstSubresource.layerCount = 1;
		copyRegion.dstOffset = vk::Offset3D{ 0, 0, 0 };
		copyRegion.extent.width = gpuContext->getSwapchainExtent().width;
		copyRegion.extent.height = gpuContext->getSwapchainExtent().height;
		copyRegion.extent.depth = 1;

		transferCommandBuffer.copyImage(
			basePass->getImage()->getHandle(), vk::ImageLayout::eTransferSrcOptimal,
			gpuContext->getSwapchainImages()[swapChainIndex]->getHandle(), vk::ImageLayout::eTransferDstOptimal,
			{ copyRegion });

		gpuContext->transferImage(
			transferCommandBuffer,
			vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eBottomOfPipe,
			vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eNone,
			vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR,
			gpuContext->getSwapchainImages()[swapChainIndex]);

		gpuContext->transferImage(
			transferCommandBuffer,
			vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eBottomOfPipe,
			vk::AccessFlagBits::eTransferRead, vk::AccessFlagBits::eNone,
			vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eColorAttachmentOptimal,
			basePass->getImage());

		transferCommandBuffer.end();

		gpuContext->submit(
			CommandType::Transfer,
			{ imageAvailableSemaphore, renderFinishedSemaphore },
			{ vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput },
			{ transferCommandBuffer },
			{ transferFinishedSemaphore },
			VK_NULL_HANDLE);


		// Present the image to the screen
		gpuContext->present(swapChainIndex, { transferFinishedSemaphore });

		// Wait for device to idle before the next frame (you can optimize this further)
		gpuContext->getDevice()->getHandle().waitIdle();
	}
}

void Application::close()
{
	gpuContext->returnSemaphore(renderFinishedSemaphore);
	gpuContext->returnSemaphore(imageAvailableSemaphore);
	gpuContext->returnSemaphore(transferFinishedSemaphore);
	gpuContext->returnFence(fence);

	delete basePass;
}
