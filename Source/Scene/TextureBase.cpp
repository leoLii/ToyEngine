#include "TextureBase.hpp"

#include "Common/Logging.hpp"

TextureBase::TextureBase(const char* path)
	:path{ path }
{
    KTX_error_code result = ktxTexture2_CreateFromNamedFile(path, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &insideTexture);
    if (result != KTX_SUCCESS) {
        LOGE("Failed to load KTX file: {}", path);
    }
}

TextureBase::~TextureBase()
{
    if (insideTexture) {
        ktxTexture2_Destroy(insideTexture);  // 使用通用销毁函数
        insideTexture = nullptr;
    }
}

