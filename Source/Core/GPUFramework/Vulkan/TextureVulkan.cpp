#include "TextureVulkan.hpp"

#include "Common/Logging.hpp"

TextureVulkan::TextureVulkan(const char* path, const GPUContext& gpuContext)
	:TextureBase{ path },
     gpuContext{ gpuContext }
{
    ktxTexture2_TranscodeBasis(insideTexture, KTX_TF_BC7_M6_OPAQUE_ONLY, KTX_TF_HIGH_QUALITY);
    ktxVulkanDeviceInfo_Construct(
        &kvdi,
        gpuContext.getDevice()->getUsingGPU(), 
        gpuContext.getDevice()->getHandle(), 
        gpuContext.getDevice()->getTransferQueue(),
        gpuContext.getCommandPool(), nullptr);

    auto ktxresult = ktxTexture2_VkUpload(insideTexture, &kvdi, &vulkanTexture);
    if (KTX_SUCCESS != ktxresult) {
        LOGE("ktxTexture_VkUpload failed: {}", ktxresult);
    }

    char* pValue;
    uint32_t valueLen;
    if (KTX_SUCCESS == ktxHashList_FindValue(&insideTexture->kvDataHead,
        KTX_ORIENTATION_KEY,
        &valueLen, (void**)&pValue))
    {
        char s, t;

        if (sscanf(pValue, KTX_ORIENTATION2_FMT, &s, &t) == 2) {
            if (s == 'l') auto sign_s = -1;
            if (t == 'u') auto sign_t = -1;
        }
    }
}

TextureVulkan::~TextureVulkan()
{
    ktxVulkanTexture_Destruct(&vulkanTexture, gpuContext.getDevice()->getHandle(), nullptr);
    ktxVulkanDeviceInfo_Destruct(&kvdi);
}
