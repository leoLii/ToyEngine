#pragma once

#include "Texture.hpp"

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
    Texture* pTexture;                      ///< Valid texture object when state is 'Loaded', or nullptr if loading failed.

    bool isValid() const { return state != TextureState::Invalid; }
};

class TextureManager {
public:
	TextureManager(uint64_t = 1024);
	~TextureManager();

	void createTextureReference(std::vector<const char*>&&, const GPUContext& gpuContext);

	Texture* findTexture(const char*);

protected:
	std::unordered_map<const char*, TextureDesc> mTextureDescs;
	const uint64_t mMaxTextureCount;

private:
	void loadFromDisk(const GPUContext&);
};