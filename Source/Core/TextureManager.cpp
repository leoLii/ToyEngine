#include "TextureManager.hpp"

#include "Core/GPUFramework/Vulkan/TextureVulkan.hpp"

TextureManager::TextureManager(uint64_t maxTextureCount)
	: mMaxTextureCount{ maxTextureCount }
{
	mTextureDescs.reserve(maxTextureCount);
}

TextureManager::~TextureManager()
{
	mTextureDescs.clear();
}

void TextureManager::createTextureReference(std::vector<const char*>&& paths, const GPUContext& gpuContext)
{
	for (auto path : paths) {
		auto texture = new TextureVulkan{ path, gpuContext };
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
