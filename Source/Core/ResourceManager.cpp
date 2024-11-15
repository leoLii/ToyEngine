#include "ResourceManager.hpp"
#include "Common/Logging.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

ResourceManager::ResourceManager(const GPUContext& gpuContext)
	:gpuContext{ gpuContext }
{
	loadShaders("C:/Users/lihan/Desktop/workspace/ToyEngine/Shader");
	loadPipelineCaches("C:/Users/lihan/Desktop/workspace/ToyEngine/PipelineCache");
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

	destroyShaders();
}

const ShaderModule* ResourceManager::findShader(const std::string& name) const
{
	auto it = shaderModules.find(name);
	if (it != shaderModules.end()) {
		return it->second;
	}
	else {
		LOGE("ShaderModule{} not found", name);
		return nullptr;
	}
}

vk::PipelineCache ResourceManager::findPipelineCache(const std::string& name) const
{
	auto it = pipelineCaches.find(name);
	if (it != pipelineCaches.end()) {
		return it->second;
	}
	else {
		LOGE("PipelineCache{} not found", name);
		return nullptr;
	}
}

ImageView* ResourceManager::createImageView(Image* image, ImageViewInfo viewInfo)
{
	auto view = new ImageView(gpuContext.getDeviceRef(), image, viewInfo);
	imageViews.push_back(view);
	return view;
}

Buffer* ResourceManager::createBuffer(
	uint64_t size,
	vk::BufferUsageFlags bufferUsage,
	VmaMemoryUsage memoryUsage,
	bool mapped)
{
	auto buffer = new Buffer{ gpuContext.getDeviceRef(), size, bufferUsage, memoryUsage, mapped };
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

void ResourceManager::loadShaders(const std::string& dir)
{
	for (const auto& entry : std::filesystem::directory_iterator(dir)) {
		if (entry.is_regular_file()) {
			std::ifstream file(entry.path(), std::ios::in | std::ios::binary);
			if (file) {
				std::ostringstream content;
				content << file.rdbuf();
				std::string extension = entry.path().extension().string();
				std::string name = entry.path().filename().string();
				if (!extension.compare(".vert")) {
					shaderModules[name] = new ShaderModule(*gpuContext.getDevice(), vk::ShaderStageFlagBits::eVertex, content.str());
				}
				else if (!extension.compare(".frag")) {
					shaderModules[name] = new ShaderModule(*gpuContext.getDevice(), vk::ShaderStageFlagBits::eFragment, content.str());
				}
				else if (!extension.compare(".comp")) {
					shaderModules[name] = new ShaderModule(*gpuContext.getDevice(), vk::ShaderStageFlagBits::eCompute, content.str());
				}
				else {
					continue;
				}
			}
			else {
				LOGE("无法打开文件:{}", entry.path().string());
			}
		}
	}
}

void ResourceManager::destroyShaders()
{
	for (auto shader : shaderModules) {
		delete shader.second;
	}
	shaderModules.clear();
}

void ResourceManager::loadPipelineCaches(const std::string& dir)
{
	for (const auto& entry : std::filesystem::directory_iterator(dir)) {
		if (entry.is_regular_file()) {
			std::ifstream cacheFile(entry.path(), std::ios::in | std::ios::binary);
			if (cacheFile) {
				std::string name = entry.path().filename().string();
				std::vector<uint8_t> loadedCacheData((std::istreambuf_iterator<char>(cacheFile)),
					std::istreambuf_iterator<char>());
				cacheFile.close();
				// 创建包含初始数据的Pipeline Cache
				vk::PipelineCacheCreateInfo pipelineCacheCreateInfo = {};
				pipelineCacheCreateInfo.initialDataSize = loadedCacheData.size();
				pipelineCacheCreateInfo.pInitialData = loadedCacheData.data();

				vk::PipelineCache pipelineCache;
				vk::Result result = gpuContext.getDevice()->getHandle().createPipelineCache(&pipelineCacheCreateInfo, nullptr, &pipelineCache);
				if (result != vk::Result::eSuccess) {
					LOGE("PipelineCache load failed:{}", entry.path().string());
				}
				pipelineCaches[name] = pipelineCache;
			}
		}
	}
}

void ResourceManager::destroyPipelineCaches()
{
	for (auto pipelineCache : pipelineCaches) {
		delete pipelineCache.second;
	}
	pipelineCaches.clear();
}
