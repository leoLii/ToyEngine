#pragma once

#include "../Source/Core/GPUFramework/Vulkan/VkCommon.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../ThirdParty/stb_image.h"

#include <string>
#include <vector>

class Texture {
public:
	Texture(const char*);
	~Texture();

	void* getData();

protected:
	const char* path;
	unsigned char* data;
	int32_t width;
	int32_t height;
	int32_t channels;
};