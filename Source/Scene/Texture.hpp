#pragma once

#include "../Source/Core/GPUFramework/Vulkan/VkCommon.hpp"

#include <ktx.h>

#include <string>
#include <vector>

class Texture {
public:
	Texture(const char*);
	~Texture();

	ktxTexture2* getktxTexture();

protected:
	const char* path;
	ktxTexture2* insideTexture;
	
};