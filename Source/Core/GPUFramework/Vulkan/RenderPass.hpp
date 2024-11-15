#pragma once

#include "VkCommon.hpp"

#include <vector>

class Device;
class Image;
class ImageView;


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