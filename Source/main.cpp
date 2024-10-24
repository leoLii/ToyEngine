#include "Common/Logging.hpp"

#include "Platform/Application.hpp"

#include "Scene/scene.hpp"
#include "Scene/Node.hpp"
#include "Scene/Components/Camera.hpp"

int main() {
/////////////////////////////////////////////////////////////////////
    logging::init();

    Scene scene{};
    auto camera = scene.getCamera();
    camera->lookAt(Vec3(5.0f, 5.0f, -5.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f));
    std::string path{ "C:/Users/lihan/Desktop/workspace/ToyEngine/Resource/cube/cube.fbx" };
    auto node1 = scene.loadModel(path);
    auto node2 = scene.loadModel(path);
    Mat4 matrix{ 1.0 };
    matrix = glm::translate(matrix, Vec3(1, 1, 2));
    matrix = glm::scale(matrix, Vec3(0.5, 2, 1));
    node1->setTransform(matrix);
    scene.getRootNode()->addChild(node1);
    scene.getRootNode()->addChild(node2);
    scene.collectMeshes();

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
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    
    ApplicationConfig config{ "ToyEngine", 1920, 1080, layers, extensions };

    app.init(config, &scene);
    
    app.run();

    app.close();

    return 0;
}
