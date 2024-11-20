#include "Texture.hpp"

#include "Common/Logging.hpp"

Texture::Texture(const char* path)
	:path{ path }
{
    KTX_error_code result = ktxTexture2_CreateFromNamedFile(path, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &insideTexture);
    if (result != KTX_SUCCESS) {
        LOGE("Failed to load KTX file: {}", path);
    }
}

Texture::~Texture()
{
    if (insideTexture) {
        ktxTexture_Destroy((ktxTexture*)insideTexture);  // 使用通用销毁函数
        insideTexture = nullptr;
    }
}
