#pragma once

#include "VkCommon.hpp"

#include <vector>

class Device;
class Image;
class ImageView;

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