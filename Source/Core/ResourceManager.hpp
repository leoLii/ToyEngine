#pragma once

#include "GPUFramework/GPUContext.hpp"

#include <vector>
#include <unordered_map>
#include <string>

class ResourceManager {
public:
	ResourceManager(const GPUContext&);

	~ResourceManager();

	ImageView* createImageView(Image*, ImageViewInfo = ImageViewInfo{});

	Buffer* createBuffer(uint64_t, vk::BufferUsageFlags);

	Image* createImage(ImageInfo);

	vk::Sampler createSampler(
		vk::Filter = vk::Filter::eLinear, vk::Filter = vk::Filter::eLinear,
		vk::SamplerAddressMode = vk::SamplerAddressMode::eClampToEdge,
		bool = false, float = 0.0,
		vk::BorderColor = vk::BorderColor::eIntOpaqueBlack);

	void createAttachment(std::string, ImageInfo, ImageViewInfo, AttachmentInfo);

	Attachment* getAttachment(std::string);

protected:
	const GPUContext& gpuContext;

	std::vector<Buffer*> buffers;
	std::vector<Image*> images;
	std::vector<ImageView*> imageViews;
	std::unordered_map<std::string, Attachment*> attachments;
	std::vector<vk::Sampler> samplers;
};