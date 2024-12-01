#include "RenderContext.hpp"

#include "Core/GPUFramework/GPUContext.hpp"
#include "Core/Passes/FrustumCull.hpp"
#include "Core/Passes/GBuffer.hpp"
#include "Core/Passes/Lighting.hpp"
#include "Core/Passes/Taa.hpp"
#include "Core/ResourceManager.hpp"
#include "Scene/Scene.hpp"

RenderContext::RenderContext()
	:gpuContext{ GPUContext::GetSingleton()}
	, resourceManager{ ResourceManager::GetSingleton() }
{
}

RenderContext::~RenderContext()
{
}

void RenderContext::prepare(const Scene* scene)
{
	createAttachments(1920, 1080);

	fence = gpuContext.requestFence();

	imageAvailableSemaphore = gpuContext.requestSemaphore();
	renderFinishedSemaphore = gpuContext.requestSemaphore();
	transferFinishedSemaphore = gpuContext.requestSemaphore();

	renderCommandBuffer = gpuContext.requestCommandBuffer(CommandType::Graphics, vk::CommandBufferLevel::ePrimary);
	transferCommandBuffer = gpuContext.requestCommandBuffer(CommandType::Graphics, vk::CommandBufferLevel::ePrimary);

	gBufferPass = new GBufferPass{ scene };
	lightingPass = new LightingPass{ scene };
	taaPass = new TaaPass{ scene };
	cullPass = new FrustumCullPass{ scene };

	gBufferPass->prepare();
	lightingPass->prepare();
	taaPass->prepare();
	cullPass->prepare();
}

void RenderContext::render(uint64_t frameIndex)
{
	gBufferPass->update(frameIndex);
	lightingPass->update(frameIndex);
	cullPass->update(frameIndex);

	gpuContext.waitForFences(fence);
	gpuContext.resetFences(fence);
	auto acquieResult = gpuContext.acquireNextImage(imageAvailableSemaphore, VK_NULL_HANDLE);
	uint32_t swapChainIndex = std::get<1>(acquieResult);
	//gpuContext->getDevice()->getHandle().resetCommandPool(gpuContext->getCommandPool(), vk::CommandPoolResetFlagBits::eReleaseResources);
	if (std::get<0>(acquieResult) == vk::Result::eSuccess) {
		{
			vk::CommandBufferBeginInfo beginInfo;
			beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
			renderCommandBuffer.begin(beginInfo);

			gpuContext.imageBarrier(
				renderCommandBuffer,
				vk::PipelineStageFlagBits2::eTopOfPipe, vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eBlit, vk::AccessFlagBits2::eTransferWrite,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
				gpuContext.getSwapchainImages()[swapChainIndex]);

			cullPass->record(renderCommandBuffer);

			gBufferPass->record(renderCommandBuffer);

			lightingPass->record(renderCommandBuffer);

			taaPass->record(renderCommandBuffer);

			gpuContext.imageBarrier(
				renderCommandBuffer,
				vk::PipelineStageFlagBits2::eFragmentShader, vk::AccessFlagBits2::eShaderWrite, 
				vk::PipelineStageFlagBits2::eBlit, vk::AccessFlagBits2::eTransferRead,
				vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral,
				resourceManager.getAttachment("taaOutput")->image);

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
				resourceManager.getAttachment("taaOutput")->image->getHandle(), vk::ImageLayout::eGeneral,
				gpuContext.getSwapchainImages()[swapChainIndex]->getHandle(), vk::ImageLayout::eTransferDstOptimal,
				{ blit }, vk::Filter::eLinear);

			gpuContext.imageBarrier(
				renderCommandBuffer,
				vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite, 
				vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eNone,
				vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR,
				gpuContext.getSwapchainImages()[swapChainIndex]);

			renderCommandBuffer.end();
			gpuContext.submit(
				CommandType::Graphics,
				{ imageAvailableSemaphore },
				{ vk::PipelineStageFlagBits::eAllGraphics },
				{ renderCommandBuffer },
				{ renderFinishedSemaphore },
				fence);
		}

		// Present the image to the screen
		gpuContext.present(swapChainIndex, { renderFinishedSemaphore });
	}
}

void RenderContext::clear()
{
	gpuContext.getDevice()->getHandle().waitIdle();
	gpuContext.returnSemaphore(renderFinishedSemaphore);
	gpuContext.returnSemaphore(imageAvailableSemaphore);
	gpuContext.returnSemaphore(transferFinishedSemaphore);
	gpuContext.returnFence(fence);

	delete gBufferPass;
	delete lightingPass;
	delete taaPass;
	delete cullPass;
}

void RenderContext::createAttachments(uint32_t renderWidth, uint32_t renderHeight)
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

		resourceManager.createAttachment("gPosition", imageInfo, imageViewInfo, attachmentInfo);
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

		resourceManager.createAttachment("gAlbedo", imageInfo, imageViewInfo, attachmentInfo);
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

		resourceManager.createAttachment("gNormal", imageInfo, imageViewInfo, attachmentInfo);
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

		resourceManager.createAttachment("gARM", imageInfo, imageViewInfo, attachmentInfo);
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

		resourceManager.createAttachment("gVelocity", imageInfo, imageViewInfo, attachmentInfo);
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

		resourceManager.createAttachment("gDepth", imageInfo, imageViewInfo, attachmentInfo);
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

		resourceManager.createAttachment("ColorBuffer", imageInfo, imageViewInfo, attachmentInfo);
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

		resourceManager.createAttachment("taaOutput", imageInfo, imageViewInfo, attachmentInfo);
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

		resourceManager.createAttachment("taaHistory", imageInfo, imageViewInfo, attachmentInfo);
	}

//	{
//		vk::Format format = vk::Format::eD32Sfloat;
//		ImageInfo imageInfo{};
//		imageInfo.format = format;
//		imageInfo.extent = vk::Extent3D{ renderWidth, renderHeight, 1 };
//		imageInfo.mipmapLevel = 1;
//		imageInfo.usage =
//			vk::ImageUsageFlagBits::eInputAttachment |
//			vk::ImageUsageFlagBits::eTransferDst |
//			vk::ImageUsageFlagBits::eSampled;
//		imageInfo.queueFamilyCount = 1;
//		imageInfo.pQueueFamilyIndices = { 0 };
//
//		ImageViewInfo imageViewInfo{};
//		imageViewInfo.format = format;
//
//		AttachmentInfo attachmentInfo{};
//		attachmentInfo.format = format;
//		//attachmentInfo.loadOp = vk::AttachmentLoadOp::eLoad;
//#ifdef REVERSE_DEPTH
//		attachmentInfo.clearValue = vk::ClearDepthStencilValue{ 0u, 0u };
//#elif
//		attachmentInfo.clearValue = vk::ClearDepthStencilValue{ 1u, 0u };
//#endif // REVERSE_DEPTH
//
//		resourceManager.createAttachment("DepthPyrimid", imageInfo, imageViewInfo, attachmentInfo);
//	}
}

