#pragma once

#include "VkCommon.hpp"

#include <vector>

class Device;
class RenderPass;

class Framebuffer {
public:
	Framebuffer() = delete;

	Framebuffer(const Device&, const RenderPass&, const std::vector<vk::ImageView>&);

	~Framebuffer();

	vk::Framebuffer getHandle() const;

protected:
	const Device& device;

	const RenderPass& renderPass;

	std::vector<vk::ImageView> imageViews;

	vk::Framebuffer handle;
};