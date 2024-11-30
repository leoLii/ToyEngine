#include "Application.hpp"

#include "Scene/Scene.hpp"
#include "Scene/Components/Camera.hpp"

#include "Core/Passes/GBuffer.hpp"
#include "Core/Passes/Lighting.hpp"
#include "Core/Passes/Taa.hpp"
#include "Core/Passes/temporalAntiAliasing.hpp"
#include "Core/Passes/FrustumCull.hpp"
#include "Core/GPUFramework/Vulkan/TextureVulkan.hpp"
#include "Core/TextureManager.hpp"

#include <cstddef>
#include <functional>
#include <thread>
#include <future>

void Application::init(ApplicationConfig& config, Scene* scene)
{
	window = std::make_unique<Window>(config.name, config.width, config.height);
	auto windowExtensions = Window::requireWindowExtensions();
	config.extensions.insert(config.extensions.end(), windowExtensions.begin(), windowExtensions.end());
	
	GPUContext::GetSingleton().init(config.name, config.layers, config.extensions, window.get());

	createAttachments(1920, 1080);

	//auto texture = TextureVulkan{ , *gpuContext };
	fence = GPUContext::GetSingleton().requestFence();

	std::vector<const char*> texturePath = {
		"C:/Users/lihan/Desktop/workspace/ToyEngine/Resource/cat/textures/diffuse.ktx2",
		"C:/Users/lihan/Desktop/workspace/ToyEngine/Resource/cat/textures/normal.ktx2",
		"C:/Users/lihan/Desktop/workspace/ToyEngine/Resource/cat/textures/metal.ktx2",
		"C:/Users/lihan/Desktop/workspace/ToyEngine/Resource/cat/textures/roughness.ktx2"
	};
	
	TextureManager::GetSingleton().createTextureReference(std::move(texturePath));
	
	this->scene = scene;

	Vec2 size(config.width, config.height);
	gBufferPass = new GBufferPass{ scene };
	lightingPass = new LightingPass{ scene };
	taaPass = new TaaPass{ scene };
	cullPass = new FrustumCullPass{ scene };

	imageAvailableSemaphore = GPUContext::GetSingleton().requestSemaphore();
	renderFinishedSemaphore = GPUContext::GetSingleton().requestSemaphore();
	transferFinishedSemaphore = GPUContext::GetSingleton().requestSemaphore();

	renderCommandBuffer = GPUContext::GetSingleton().requestCommandBuffer(CommandType::Graphics, vk::CommandBufferLevel::ePrimary);
	transferCommandBuffer = GPUContext::GetSingleton().requestCommandBuffer(CommandType::Graphics, vk::CommandBufferLevel::ePrimary);

	gBufferPass->prepare();
	lightingPass->prepare();
	taaPass->prepare();
	cullPass->prepare();
}

void Application::run()
{
	while (!window->shouldClose()) {

		++frameIndex;

		scene->update(frameIndex);
		gBufferPass->update(frameIndex);
		lightingPass->update(frameIndex);
		cullPass->update(frameIndex);

		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
		deltaTime = std::chrono::duration<double, std::milli>(now - lastFrameTime).count() / 1000.0;
		lastFrameTime = now;

		window->pollEvents();

		GPUContext::GetSingleton().waitForFences(fence);
		GPUContext::GetSingleton().resetFences(fence);
		auto acquieResult = GPUContext::GetSingleton().acquireNextImage(imageAvailableSemaphore, VK_NULL_HANDLE);
		uint32_t swapChainIndex = std::get<1>(acquieResult);
		//gpuContext->getDevice()->getHandle().resetCommandPool(gpuContext->getCommandPool(), vk::CommandPoolResetFlagBits::eReleaseResources);
		if (std::get<0>(acquieResult) == vk::Result::eSuccess) {
			{
				vk::CommandBufferBeginInfo beginInfo;
				beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
				renderCommandBuffer.begin(beginInfo);

				GPUContext::GetSingleton().imageBarrier(
					renderCommandBuffer,
					vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eBlit,
					vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eTransferWrite,
					vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
					GPUContext::GetSingleton().getSwapchainImages()[swapChainIndex]);

				cullPass->record(renderCommandBuffer);

				gBufferPass->record(renderCommandBuffer);

				lightingPass->record(renderCommandBuffer);

				taaPass->record(renderCommandBuffer);

				GPUContext::GetSingleton().imageBarrier(
					renderCommandBuffer,
					vk::PipelineStageFlagBits2::eFragmentShader, vk::PipelineStageFlagBits2::eBlit,
					vk::AccessFlagBits2::eShaderWrite, vk::AccessFlagBits2::eTransferRead,
					vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
					ResourceManager::GetSingleton().getAttachment("taaOutput")->image);

				vk::ImageBlit blit;
				blit.srcOffsets[0] = vk::Offset3D{ 0, 0, 0 };
				blit.srcOffsets[1] = vk::Offset3D{ 1920, 1080, 1 };
				blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
				blit.srcSubresource.mipLevel = 0;
				blit.srcSubresource.layerCount = 1;
				blit.dstOffsets[0] = vk::Offset3D{ 0, 0, 0 };
				blit.dstOffsets[1] = vk::Offset3D{ 1920, 1080, 1 };
				blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
				blit.dstSubresource.mipLevel = 0;
				blit.dstSubresource.layerCount = 1;

				renderCommandBuffer.blitImage(
					ResourceManager::GetSingleton().getAttachment("taaOutput")->image->getHandle(), vk::ImageLayout::eGeneral,
					GPUContext::GetSingleton().getSwapchainImages()[swapChainIndex]->getHandle(), vk::ImageLayout::eTransferDstOptimal,
					{ blit }, vk::Filter::eLinear);

				GPUContext::GetSingleton().imageBarrier(
					renderCommandBuffer,
					vk::PipelineStageFlagBits2::eTransfer, vk::PipelineStageFlagBits2::eBottomOfPipe,
					vk::AccessFlagBits2::eTransferWrite, vk::AccessFlagBits2::eNone,
					vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR,
					GPUContext::GetSingleton().getSwapchainImages()[swapChainIndex]);

				renderCommandBuffer.end();
				GPUContext::GetSingleton().submit(
					CommandType::Graphics,
					{imageAvailableSemaphore},
					{ vk::PipelineStageFlagBits::eAllGraphics },
					{ renderCommandBuffer },
					{ renderFinishedSemaphore },
					fence);
			}

			// Present the image to the screen
			GPUContext::GetSingleton().present(swapChainIndex, { renderFinishedSemaphore });
		}
	}
}

void Application::close()
{
	GPUContext::GetSingleton().getDevice()->getHandle().waitIdle();
	GPUContext::GetSingleton().returnSemaphore(renderFinishedSemaphore);
	GPUContext::GetSingleton().returnSemaphore(imageAvailableSemaphore);
	GPUContext::GetSingleton().returnSemaphore(transferFinishedSemaphore);
	GPUContext::GetSingleton().returnFence(fence);

	delete gBufferPass;
	delete lightingPass;
	delete taaPass;
	delete cullPass;
}

void Application::createAttachments(uint32_t renderWidth, uint32_t renderHeight)
{
	{
		vk::Format format = vk::Format::eR16G16B16A16Sfloat;
		ImageInfo imageInfo{};
		imageInfo.format = format;
		imageInfo.extent = vk::Extent3D{ renderWidth, renderHeight, 1 };
		imageInfo.usage =
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eSampled |
			vk::ImageUsageFlagBits::eInputAttachment;
		imageInfo.queueFamilyCount = 1;
		imageInfo.pQueueFamilyIndices = { 0 };

		ImageViewInfo imageViewInfo{};
		imageViewInfo.format = format;

		AttachmentInfo attachmentInfo;
		attachmentInfo.format = format;

		ResourceManager::GetSingleton().createAttachment("gPosition", imageInfo, imageViewInfo, attachmentInfo);
	}

	{
		vk::Format format = vk::Format::eR8G8B8A8Unorm;
		ImageInfo imageInfo{};
		imageInfo.format = format;
		imageInfo.extent = vk::Extent3D{ renderWidth, renderHeight, 1 };
		imageInfo.usage =
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eSampled |
			vk::ImageUsageFlagBits::eInputAttachment;
		imageInfo.queueFamilyCount = 1;
		imageInfo.pQueueFamilyIndices = { 0 };

		ImageViewInfo imageViewInfo{};
		imageViewInfo.format = format;

		AttachmentInfo attachmentInfo{};
		attachmentInfo.format = format;

		ResourceManager::GetSingleton().createAttachment("gAlbedo", imageInfo, imageViewInfo, attachmentInfo);
	}

	{
		vk::Format format = vk::Format::eR16G16B16A16Sfloat;
		ImageInfo imageInfo{};
		imageInfo.format = format;
		imageInfo.extent = vk::Extent3D{ renderWidth, renderHeight, 1 };
		imageInfo.usage =
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eSampled |
			vk::ImageUsageFlagBits::eInputAttachment;
		imageInfo.queueFamilyCount = 1;
		imageInfo.pQueueFamilyIndices = { 0 };

		ImageViewInfo imageViewInfo{};
		imageViewInfo.format = format;

		AttachmentInfo attachmentInfo{};
		attachmentInfo.format = format;

		ResourceManager::GetSingleton().createAttachment("gNormal", imageInfo, imageViewInfo, attachmentInfo);
	}

	{
		vk::Format format = vk::Format::eR16G16B16A16Snorm;
		ImageInfo imageInfo{};
		imageInfo.format = format;
		imageInfo.extent = vk::Extent3D{ renderWidth, renderHeight, 1 };
		imageInfo.usage =
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eSampled |
			vk::ImageUsageFlagBits::eInputAttachment;
		imageInfo.queueFamilyCount = 1;
		imageInfo.pQueueFamilyIndices = { 0 };

		ImageViewInfo imageViewInfo{};
		imageViewInfo.format = format;

		AttachmentInfo attachmentInfo{};
		attachmentInfo.format = format;

		ResourceManager::GetSingleton().createAttachment("gARM", imageInfo, imageViewInfo, attachmentInfo);
	}

	{
		vk::Format format = vk::Format::eR16G16Snorm;
		ImageInfo imageInfo{};
		imageInfo.format = format;
		imageInfo.extent = vk::Extent3D{ renderWidth, renderHeight, 1 };
		imageInfo.usage =
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eSampled |
			vk::ImageUsageFlagBits::eInputAttachment;
		imageInfo.queueFamilyCount = 1;
		imageInfo.pQueueFamilyIndices = { 0 };

		ImageViewInfo imageViewInfo{};
		imageViewInfo.format = format;

		AttachmentInfo attachmentInfo{};
		attachmentInfo.format = format;

		ResourceManager::GetSingleton().createAttachment("gVelocity", imageInfo, imageViewInfo, attachmentInfo);
	}

	{
		vk::Format format = vk::Format::eD32Sfloat;
		ImageInfo imageInfo{};
		imageInfo.format = format;
		imageInfo.extent = vk::Extent3D{ renderWidth, renderHeight, 1 };
		imageInfo.usage =
			vk::ImageUsageFlagBits::eDepthStencilAttachment |
			vk::ImageUsageFlagBits::eSampled |
			vk::ImageUsageFlagBits::eInputAttachment;
		imageInfo.queueFamilyCount = 1;
		imageInfo.pQueueFamilyIndices = { 0 };

		ImageViewInfo imageViewInfo{};
		imageViewInfo.format = format;
		imageViewInfo.range = vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 };

		AttachmentInfo attachmentInfo{};
		attachmentInfo.format = format;

#ifdef REVERSE_DEPTH
		attachmentInfo.clearValue = vk::ClearDepthStencilValue{ 0u, 0u };
#elif
		attachmentInfo.clearValue = vk::ClearDepthStencilValue{ 1u, 0u };
#endif // REVERSE_DEPTH

		ResourceManager::GetSingleton().createAttachment("gDepth", imageInfo, imageViewInfo, attachmentInfo);
	}

	{
		vk::Format format = vk::Format::eR8G8B8A8Unorm;
		ImageInfo imageInfo{};
		imageInfo.format = format;
		imageInfo.extent = vk::Extent3D{ renderWidth, renderHeight, 1 };
		imageInfo.usage =
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eSampled;
		imageInfo.queueFamilyCount = 1;
		imageInfo.pQueueFamilyIndices = { 0 };

		ImageViewInfo imageViewInfo{};
		imageViewInfo.format = format;

		AttachmentInfo attachmentInfo{};
		attachmentInfo.format = format;
		attachmentInfo.clearValue = vk::ClearColorValue{ 0u, 0u, 0u, 0u };

		ResourceManager::GetSingleton().createAttachment("ColorBuffer", imageInfo, imageViewInfo, attachmentInfo);
	}

	{
		vk::Format format = vk::Format::eR8G8B8A8Unorm;
		ImageInfo imageInfo{};
		imageInfo.format = format;
		imageInfo.extent = vk::Extent3D{ renderWidth, renderHeight, 1 };
		imageInfo.usage =
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eStorage |
			vk::ImageUsageFlagBits::eTransferSrc;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.queueFamilyCount = 1;
		imageInfo.pQueueFamilyIndices = { 0 };

		ImageViewInfo imageViewInfo{};
		imageViewInfo.format = format;

		AttachmentInfo attachmentInfo{};
		attachmentInfo.format = format;
		attachmentInfo.clearValue = vk::ClearColorValue{ 0u, 0u, 0u, 0u };

		ResourceManager::GetSingleton().createAttachment("taaOutput", imageInfo, imageViewInfo, attachmentInfo);
	}

	{
		vk::Format format = vk::Format::eR8G8B8A8Unorm;
		ImageInfo imageInfo{};
		imageInfo.format = format;
		imageInfo.extent = vk::Extent3D{ renderWidth, renderHeight, 1 };
		imageInfo.usage =
			vk::ImageUsageFlagBits::eInputAttachment |
			vk::ImageUsageFlagBits::eTransferDst |
			vk::ImageUsageFlagBits::eSampled;
		imageInfo.queueFamilyCount = 1;
		imageInfo.pQueueFamilyIndices = { 0 };

		ImageViewInfo imageViewInfo{};
		imageViewInfo.format = format;

		AttachmentInfo attachmentInfo{};
		attachmentInfo.format = format;
		attachmentInfo.clearValue = vk::ClearColorValue{ 0u, 0u, 0u, 0u };

		ResourceManager::GetSingleton().createAttachment("taaHistory", imageInfo, imageViewInfo, attachmentInfo);
	}

	{
		vk::Format format = vk::Format::eD32Sfloat;
		ImageInfo imageInfo{};
		imageInfo.format = format;
		imageInfo.extent = vk::Extent3D{ renderWidth, renderHeight, 1 };
		imageInfo.mipmapLevel = 1;
		imageInfo.usage =
			vk::ImageUsageFlagBits::eInputAttachment |
			vk::ImageUsageFlagBits::eTransferDst |
			vk::ImageUsageFlagBits::eSampled;
		imageInfo.queueFamilyCount = 1;
		imageInfo.pQueueFamilyIndices = { 0 };

		ImageViewInfo imageViewInfo{};
		imageViewInfo.format = format;

		AttachmentInfo attachmentInfo{};
		attachmentInfo.format = format;
		//attachmentInfo.loadOp = vk::AttachmentLoadOp::eLoad;
#ifdef REVERSE_DEPTH
		attachmentInfo.clearValue = vk::ClearDepthStencilValue{ 0u, 0u };
#elif
		attachmentInfo.clearValue = vk::ClearDepthStencilValue{ 1u, 0u };
#endif // REVERSE_DEPTH

		ResourceManager::GetSingleton().createAttachment("DepthPyrimid", imageInfo, imageViewInfo, attachmentInfo);
	}
}
