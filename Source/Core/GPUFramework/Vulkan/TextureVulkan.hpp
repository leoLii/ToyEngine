#pragma once

#include "Scene/Texture.hpp"

#include "VkCommon.hpp"
#include "../GPUContext.hpp"

#include <ktxvulkan.h>

class TextureVulkan : public Texture {
public:
	TextureVulkan(const char*, const GPUContext&);
	~TextureVulkan();

protected:
	const GPUContext& gpuContext;

	ktxVulkanDeviceInfo kvdi;
	ktxVulkanTexture vulkanTexture;

};