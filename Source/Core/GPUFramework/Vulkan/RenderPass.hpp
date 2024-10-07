#pragma once

#include "VkCommon.hpp"

#include <vector>

class Device;

class RenderPass {
public:
	RenderPass() = delete;
	RenderPass(const Device&);

	~RenderPass();

	vk::RenderPass getHandle() {
		return this->handle;
	}

protected:
	const Device& device;

	vk::RenderPass handle;

	std::vector<vk::Image> attachments;
};