#include "Common/Logging.hpp"

#include "Platform/Application.hpp"

#include "Scene/Loader.hpp"

int main() {
/////////////////////////////////////////////////////////////////////
    logging::init();

    AssimpLoader loader;
    loader.loadModel("C:/Users/lihan/Desktop/workspace/ToyEngine/Resource/nanosuit/nanosuit.obj");

    Application app{};

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
    
    ApplicationConfig config{ "ToyEngine", 1920, 1080, layers, extensions };

    app.init(config);
    
    app.run(0.0);

    app.close();

    return 0;
}
