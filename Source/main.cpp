#include "Header.hpp"
#include "Common/Logging.hpp"


int main() {
    logging::init();

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

    auto vertShaderModule = gpuContext->findShader("triangle.vert");
    auto fragShaderModule = gpuContext->findShader("triangle.frag");

    std::vector<vk::ShaderModule> modules;
    modules.push_back(vertShaderModule->getHandle());
    modules.push_back(fragShaderModule->getHandle());
    graphicsPipeline = new GraphicsPipeline(*gpuContext->getDevice(), *renderPass, modules);

    createFramebuffers();
    
    commandPool = new CommandPool(*gpuContext->getDevice(), 0, MAX_FRAMES_IN_FLIGHT);
    
    while(!window->shouldClose()) {
        window->pollEvents();
        drawFrame();
    }

    cleanup();

    return 0;
}
