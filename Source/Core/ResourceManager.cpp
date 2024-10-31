#include "ResourceManager.hpp"

ResourceManager::ResourceManager(const GPUContext& gpuContext)
	:gpuContext{ gpuContext }
{
}

ResourceManager::~ResourceManager()
{
	for (auto buffer : buffers) {
		delete buffer;
	}
	buffers.clear();

	for (auto image : images) {
		delete image;
	}
	images.clear();

	for (auto view : imageViews) {
		delete view;
	}
	imageViews.clear();

	for (auto sampler : samplers) {
		gpuContext.getDeviceRef().getHandle().destroySampler(sampler);
	}
}

ImageView* ResourceManager::createImageView(Image* image,  ImageViewInfo viewInfo)
{
	auto view = new ImageView(gpuContext.getDeviceRef(), image, viewInfo);
	imageViews.push_back(view);
	return view;
}

Buffer* ResourceManager::createBuffer(
	uint64_t size, 
	vk::BufferUsageFlags usage)
{
	auto buffer = new Buffer{ gpuContext.getDeviceRef(), size, usage };
	buffers.push_back(buffer);
	return buffer;
}

Image* ResourceManager::createImage(ImageInfo imageInfo)
{
	auto image = new Image{ gpuContext.getDeviceRef(), imageInfo };
	images.push_back(image);
	return image;
}

vk::Sampler ResourceManager::createSampler(
	vk::Filter magFilter, vk::Filter minFilter,
	vk::SamplerAddressMode addressMode,
	bool enableAnisotropy, float maxAnisotropy,
	vk::BorderColor borderColor)
{
	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo.magFilter = magFilter;
	samplerInfo.minFilter = minFilter;
	samplerInfo.addressModeU = addressMode;
	samplerInfo.addressModeV = addressMode;
	samplerInfo.addressModeW = addressMode;
	samplerInfo.borderColor = borderColor;

	samplerInfo.anisotropyEnable = enableAnisotropy;
	samplerInfo.maxAnisotropy = enableAnisotropy ? maxAnisotropy : 1.0f;

	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

	auto sampler = gpuContext.getDeviceRef().getHandle().createSampler(samplerInfo);

	samplers.push_back(sampler);

	return sampler;
}

void ResourceManager::createAttachment(
	std::string name,
	ImageInfo imageInfo, 
	ImageViewInfo viewInfo,
	AttachmentInfo attachmentInfo)
{
	auto attachment = new Attachment{};
	attachment->image = createImage(imageInfo);
	attachment->view = createImageView(attachment->image, viewInfo);
	attachment->attachmentInfo = attachmentInfo;
	attachments[name] = attachment;
}

Attachment* ResourceManager::getAttachment(std::string name)
{
	return attachments[name];
}
