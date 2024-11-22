#pragma once

#include "Common/UUID.hpp"
#include "../Source/Core/GPUFramework/Vulkan/VkCommon.hpp"

#include <ktx.h>

#include <string>
#include <vector>

class Texture {
public:
	Texture();
	Texture(const char*);
	~Texture();

	const uuid getUUID() const {
		return uid;
	}

protected:
	uuid uid;
	const char* path;
	ktxTexture2* insideTexture;
};