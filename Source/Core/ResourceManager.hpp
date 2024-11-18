#pragma once

#include "GPUFramework/GPUContext.hpp"

#include <vector>
#include <unordered_map>
#include <string>

struct AttachmentInfo {
	vk::Format format = vk::Format::eR8G8B8A8Unorm;
	vk::ImageLayout layout = vk::ImageLayout::eGeneral;
	vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear;
	vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore;
	vk::ClearValue clearValue = vk::ClearColorValue{ 0u, 0u, 0u, 0u };
};

struct Attachment {
	Image* image;
	ImageView* view;
	AttachmentInfo attachmentInfo;
};

class ResourceManager {
public:
	ResourceManager(const GPUContext&);

	~ResourceManager();

	const ShaderModule* findShader(const std::string&) const;

	vk::PipelineCache findPipelineCache(const std::string&);

	ImageView* createImageView(Image*, ImageViewInfo = ImageViewInfo{});

	Buffer* createBuffer(uint64_t, vk::BufferUsageFlags, VmaMemoryUsage=VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO, bool=true);

	Image* createImage(ImageInfo);

	vk::Sampler createSampler(
		vk::Filter = vk::Filter::eLinear, vk::Filter = vk::Filter::eLinear,
		vk::SamplerAddressMode = vk::SamplerAddressMode::eClampToEdge,
		bool = false, float = 0.0,
		vk::BorderColor = vk::BorderColor::eIntOpaqueBlack);

	void createAttachment(std::string, ImageInfo, ImageViewInfo, AttachmentInfo);

	Attachment* getAttachment(std::string);

	Buffer* createIndirectBuffer(uint64_t, vk::BufferUsageFlags, VmaMemoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO, bool = true);
	Buffer* getIndirectBuffer();

protected:
	const GPUContext& gpuContext;
	Buffer* indirectBuffer;
	std::vector<Buffer*> buffers;
	std::vector<Image*> images;
	std::vector<ImageView*> imageViews;
	std::unordered_map<std::string, Attachment*> attachments;
	std::vector<vk::Sampler> samplers;
	std::unordered_map<std::string, ShaderModule*> shaderModules;
	std::unordered_map<std::string, vk::PipelineCache> pipelineCaches;

private:
	void loadShaders(const std::string&);
	void destroyShaders();
	void loadPipelineCaches(const std::string&);
	void savePipelineCaches(const std::string&);
	void destroyPipelineCaches();
};