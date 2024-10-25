#pragma once

#include "VkCommon.hpp"

#include <vector>

class Device;
class Image;
class ImageView;

struct Attachment {
	Image* image;
	ImageView* view;
	vk::Format format;
	vk::RenderingAttachmentInfo attachmentInfo;
};

class RenderPass {
public:
	RenderPass() = delete;
	RenderPass(const Device&);

	~RenderPass();

	const vk::RenderPass getHandle() const {
		return this->handle;
	}

protected:
	const Device& device;

	vk::RenderPass handle;

	std::vector<vk::Image> attachments;
};