#include "Application.hpp"

#include "Scene/Scene.hpp"
#include "Scene/Components/Camera.hpp"

#include "Core/Passes/GBuffer.hpp"
#include "Core/Passes/Lighting.hpp"

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

	//{
	//	ImageInfo imageInfo{};
	//	imageInfo.format = vk::Format::eB8G8R8A8Srgb;
	//	imageInfo.type = vk::ImageType::e2D;
	//	imageInfo.extent = vk::Extent3D{ 960, 540, 1 };
	//	imageInfo.usage =
	//		vk::ImageUsageFlagBits::eColorAttachment |
	//		vk::ImageUsageFlagBits::eTransferSrc |
	//		vk::ImageUsageFlagBits::eSampled;
	//	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	//	imageInfo.arrayLayers = 1;
	//	imageInfo.mipmapLevel = 1;
	//	imageInfo.queueFamilyCount = 1;
	//	imageInfo.pQueueFamilyIndices = { 0 };

	//	colorAttachment->image = gpuContext->createImage(imageInfo);
	//	colorAttachment->view = gpuContext->createImageView(colorAttachment->image);
	//	colorAttachment->format = imageInfo.format;
	//	colorAttachment->attachmentInfo.imageView = colorAttachment->view->getHandle();
	//	colorAttachment->attachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
	//	colorAttachment->attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
	//	colorAttachment->attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
	//	colorAttachment->attachmentInfo.clearValue.color = vk::ClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f };
	//	colorAttachment->attachmentInfo.clearValue.depthStencil = vk::ClearDepthStencilValue{ 0u, 0u };
	//}


	gBufferPass = new GBufferPass{ gpuContext.get(), scene };
	lightingPass = new LightingPass{ gpuContext.get(), scene };

	imageAvailableSemaphore = gpuContext->requestSemaphore();
	renderFinishedSemaphore = gpuContext->requestSemaphore();
	transferFinishedSemaphore = gpuContext->requestSemaphore();

	renderCommandBuffer = gpuContext->requestCommandBuffer(CommandType::Graphics, vk::CommandBufferLevel::ePrimary);
	transferCommandBuffer = gpuContext->requestCommandBuffer(CommandType::Graphics, vk::CommandBufferLevel::ePrimary);

	gBufferPass->prepare();
	lightingPass->setAttachment(0, gBufferPass->getAttachment(0));
	lightingPass->setAttachment(1, gBufferPass->getAttachment(1));
	lightingPass->setAttachment(2, gBufferPass->getAttachment(2));
	lightingPass->setAttachment(3, gBufferPass->getAttachment(3));
	lightingPass->prepare();
}

void Application::run()
{
	while (!window->shouldClose()) {

		++frameIndex;

		scene->update(frameIndex);
		gBufferPass->update(frameIndex);
		lightingPass->update(frameIndex);

		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
		deltaTime = std::chrono::duration<double, std::milli>(now - lastFrameTime).count() / 1000.0;
		lastFrameTime = now;

		window->pollEvents();

		gpuContext->waitForFences(fence);
		gpuContext->resetFences(fence);
		auto acquieResult = gpuContext->acquireNextImage(imageAvailableSemaphore, VK_NULL_HANDLE);
		uint32_t swapChainIndex = std::get<1>(acquieResult);

		{
			vk::CommandBufferBeginInfo beginInfo;
			beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
			renderCommandBuffer.begin(beginInfo);
			gBufferPass->record(renderCommandBuffer);
			lightingPass->record(renderCommandBuffer);
			renderCommandBuffer.end();
			gpuContext->submit(
				CommandType::Graphics,
				{},
				{},
				{ renderCommandBuffer },
				{ renderFinishedSemaphore },
				VK_NULL_HANDLE);
		}
		
		
		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		transferCommandBuffer.begin(beginInfo);

		gpuContext->transferImage(
			transferCommandBuffer,
			vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eAllGraphics,
			vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite,
			vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
			gpuContext->getSwapchainImages()[swapChainIndex]);

		//gpuContext->transferImage(
		//	transferCommandBuffer,
		//	vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eTransfer,
		//	vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eTransferRead,
		//	vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal,
		//	basePass->getImage());

		vk::ImageBlit blit;
		blit.srcOffsets[0] = vk::Offset3D{ 0, 0, 0 };
		blit.srcOffsets[1] = vk::Offset3D{ 960, 540, 1 };
		blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		blit.srcSubresource.mipLevel = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = vk::Offset3D{ 0, 0, 0 };
		blit.dstOffsets[1] = vk::Offset3D{ 1920, 1080, 1 };
		blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		blit.dstSubresource.mipLevel = 0;
		blit.dstSubresource.layerCount = 1;

		transferCommandBuffer.blitImage(
			lightingPass->getAttachment()->image->getHandle(), vk::ImageLayout::eGeneral,
			gpuContext->getSwapchainImages()[swapChainIndex]->getHandle(), vk::ImageLayout::eTransferDstOptimal,
			{ blit }, vk::Filter::eLinear);

		gpuContext->transferImage(
			transferCommandBuffer,
			vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eBottomOfPipe,
			vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eNone,
			vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR,
			gpuContext->getSwapchainImages()[swapChainIndex]);

		transferCommandBuffer.end();

		gpuContext->submit(
			CommandType::Transfer,
			{ imageAvailableSemaphore, renderFinishedSemaphore },
			{ vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput },
			{ transferCommandBuffer },
			{ transferFinishedSemaphore },
			fence);

		// Present the image to the screen
		gpuContext->present(swapChainIndex, { transferFinishedSemaphore });

	}
}

void Application::close()
{
	gpuContext->getDevice()->getHandle().waitIdle();
	gpuContext->returnSemaphore(renderFinishedSemaphore);
	gpuContext->returnSemaphore(imageAvailableSemaphore);
	gpuContext->returnSemaphore(transferFinishedSemaphore);
	gpuContext->returnFence(fence);

	delete gBufferPass;
	delete lightingPass;
}
