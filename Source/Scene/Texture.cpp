#include "Texture.hpp"

#include <stdexcept>

Texture::Texture(const char* path)
	:path{ path }
{
	data = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
    if (!data) {
        throw std::runtime_error("Failed to load texture image!");
    }
}

Texture::~Texture()
{
    stbi_image_free(data);
}

void* Texture::getData()
{
    return static_cast<void*>(data);
}
