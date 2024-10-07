#include "Header.hpp"


int main() {
    // 必须要先创建window再requireExtension
    window = std::make_shared<Window>("ToyEngine", WIDTH, HEIGHT);
    std::vector<const char*> layers;
#ifdef VK_ENABLE_VALIDATION
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    std::vector<const char*> extensions;
#ifdef ARCH_OS_MAC
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif // ARCH_OS_MAC

#ifdef VK_ENABLE_VALIDATION
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    auto windowExtensions = Window::requireWindowExtensions();
    extensions.insert(extensions.end(), windowExtensions.begin(), windowExtensions.end());

    gpuContext = std::make_unique<GPUContext>(layers, extensions, window);

    auto spImages = gpuContext->getSwapchainImages();
    swapChainImages.resize(spImages.size());

    std::transform(spImages.begin(), spImages.end(), swapChainImages.begin(), [](vk::Image i)->VkImage {
        return static_cast<VkImage>(i);
        });
    swapChainImageFormat = static_cast<VkFormat>(gpuContext->getSwapchainFormat());
    swapChainExtent = static_cast<VkExtent2D>(gpuContext->getSwapchainExtent());
    
    createImageViews();
    
    renderPass = new RenderPass(*gpuContext->getDevice());

    
    auto readFile = [](const std::string& filename) -> std::vector<uint32_t> {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();

        // 确保文件大小是 4 的倍数（因为我们要读取 uint32_t 类型的数据）
        if (fileSize % 4 != 0) {
            throw std::runtime_error("file size is not a multiple of 4, cannot convert to uint32_t");
        }

        std::vector<uint32_t> buffer(fileSize / 4);

        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

        file.close();

        return buffer;
     };

    auto vertShaderCode = readFile("D:/Downloads/vert.spv");
    auto fragShaderCode = readFile("D:/Downloads/frag.spv");
    
    auto createShaderModule = [](const std::vector<char> &code) -> VkShaderModule
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(gpuContext->getDevice()->getHandle(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    };

    ShaderModule vertShaderModule{ *gpuContext->getDevice(), vk::ShaderStageFlagBits::eVertex, vertShaderCode };
    ShaderModule fragShaderModule{ *gpuContext->getDevice(), vk::ShaderStageFlagBits::eFragment, fragShaderCode };

    std::vector<vk::ShaderModule> modules;
    modules.push_back(vertShaderModule.getHandle());
    modules.push_back(fragShaderModule.getHandle());
    graphicsPipeline = new GraphicsPipeline(*gpuContext->getDevice(), *renderPass, modules);
    
    fragShaderModule.~ShaderModule();
    vertShaderModule.~ShaderModule();

    createFramebuffers();
    
    commandPool = new CommandPool(*gpuContext->getDevice(), 0, MAX_FRAMES_IN_FLIGHT);
    
    while(!window->shouldClose()) {
        window->pollEvents();
        drawFrame();
    }

    cleanup();

    return 0;
}
