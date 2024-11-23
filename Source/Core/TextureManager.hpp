#pragma once

#include "Texture.hpp"
#include "Core/GPUFramework/Vulkan/TextureVulkan.hpp"

#include <unordered_map>
#include <memory>

class GPUContext;

enum TextureState
{
	Invalid,
	Referenced,
	Loaded
};

struct TextureDesc
{
	const char* path;
    TextureState state = TextureState::Invalid; ///< Current state of the texture.
    TextureVulkan* pTexture;                      ///< Valid texture object when state is 'Loaded', or nullptr if loading failed.

    bool isValid() const { return state != TextureState::Invalid; }
};

class TextureManager {
public:
	static TextureManager& GetInstance(){
		static TextureManager instance{};
		return instance;
	}

	void createTextureReference(std::vector<const char*>&&, const GPUContext& gpuContext);

	TextureVulkan *findTexture(std::string);

protected:
	std::unordered_map<std::string, TextureDesc> mTextureDescs;
	const uint64_t mMaxTextureCount;

private:
	TextureManager(uint64_t = 1024);
	~TextureManager();

	void loadFromDisk(const GPUContext&);
};