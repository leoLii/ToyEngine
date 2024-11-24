#include "Common/Logging.hpp"

#include "Platform/Application.hpp"

#include "Scene/scene.hpp"
#include "Scene/Node.hpp"
#include "Scene/Components/Camera.hpp"
#include <random>

int main() {
/////////////////////////////////////////////////////////////////////
    logging::init();
    Scene scene{};
    auto camera = scene.getCamera();
    Vec3 cameraPosition = Vec3(2.5, 2.5, 2.5);
    camera->getAttachNode()->getTransform().setTranslate(cameraPosition);
    camera->lookAt(cameraPosition, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
    std::string path{ "C:/Users/lihan/Desktop/workspace/ToyEngine/Resource/cat/cat.fbx" };
    std::default_random_engine random_generator;
    random_generator.seed(42);
    std::uniform_real_distribution<float> distribution(1.0, 180.0);

    auto startPosition = Vec3(0.0f, 0.0f, 0.0f);
    for (int i = -5; i < 5; i++) {
        for (int j = -5; j < 5; j++) {
            auto position = startPosition + Vec3(float(i), 0.0f, float(j));
            auto node = scene.loadModel(path);
            node->setScale(Vec3(3.0f, 3.0f, 3.0f));
            node->setRotate(90.0, Vec3(-1.0f, 0.0f, 0.0f));
            float angle = distribution(random_generator);
            node->setRotate(angle, Vec3(0.0f, 1.0f, 0.0f));
            node->setTranslate(position);
            scene.getRootNode()->addChild(node);
        }
    }
    /*auto node1 = scene.loadModel(path);
    node1->setScale(Vec3(3.0f, 3.0f, 3.0f));
    node1->setRotate(90.0, Vec3(-1.0f, 0.0f, 0.0f));
    scene.getRootNode()->addChild(node1);
    auto node2 = scene.loadModel(path);
    node2->setScale(Vec3(3.0f, 3.0f, 3.0f));
    node2->setRotate(90.0, Vec3(-1.0f, 0.0f, 0.0f));
    node2->setTranslate(Vec3(2.0f, 0.0f, -2.0f));
    scene.getRootNode()->addChild(node2);*/
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
    
    ApplicationConfig config{ "ToyEngine", 1920, 1080, layers, extensions };

    app.init(config, &scene);
    
    app.run();

    app.close();

    return 0;
}
