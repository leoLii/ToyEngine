#pragma once

#include "Core/Texture.hpp"

#include "VkCommon.hpp"
#include "../GPUContext.hpp"

#include <ktxvulkan.h>

class TextureVulkan : public Texture {
public:
	TextureVulkan(const char*);
	~TextureVulkan();

	vk::ImageLayout getImageLayout() {
		return static_cast<vk::ImageLayout>(vulkanTexture.imageLayout);
	}

	vk::ImageView getImageView() {
		return view;
	}


protected:
	const GPUContext& gpuContext;

	ktxVulkanDeviceInfo kvdi;
	ktxVulkanTexture vulkanTexture;

	vk::ImageView view;
};