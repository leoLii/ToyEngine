#pragma once

#include "Scene/TextureBase.hpp"

#include "VkCommon.hpp"
#include "../GPUContext.hpp"

#include <ktxvulkan.h>

class TextureVulkan : public TextureBase {
public:
	TextureVulkan(const char*, const GPUContext&);
	~TextureVulkan();

protected:
	const GPUContext& gpuContext;

	ktxVulkanDeviceInfo kvdi;
	ktxVulkanTexture vulkanTexture;

};