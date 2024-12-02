#include "TextureManager.hpp"

#include "Core/GPUFramework/Vulkan/TextureVulkan.hpp"

TextureManager::TextureManager(uint64_t maxTextureCount)
	: mMaxTextureCount{ maxTextureCount }
{
	mTextureDescs.reserve(maxTextureCount);
}

TextureManager::~TextureManager()
{
	for (auto texture : mTextureDescs) {
		delete texture.second.pTexture;
	}
	mTextureDescs.clear();
}

void TextureManager::createTextureReference(std::vector<const char*>&& paths)
{
	for (auto path : paths) {
		auto texture = new TextureVulkan{ path };
		TextureDesc textureDesc{ path, TextureState::Loaded, texture };
		mTextureDescs.insert(std::make_pair(std::string(path), std::move(textureDesc)));
	}
}

TextureVulkan* TextureManager::findTexture(std::string path)
{
	return mTextureDescs.find(path)->second.pTexture;
}

void TextureManager::loadFromDisk(const GPUContext& gpuContext)
{

}
