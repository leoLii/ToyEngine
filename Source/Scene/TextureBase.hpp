#pragma once

#include "../Source/Core/GPUFramework/Vulkan/VkCommon.hpp"

#include <ktx.h>

#include <string>
#include <vector>

class TextureBase {
public:
	TextureBase(const char*);
	~TextureBase();

	ktxTexture2* getktxTexture();

protected:
	const char* path;
	ktxTexture2* insideTexture;
	
};