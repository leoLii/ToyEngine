#include "SceneBuilder.hpp"

#include "Scene/Scene.hpp"
#include "Scene/Components/Camera.hpp"

#include "Rendering/Material.hpp"
#include "Rendering/Passes/GBuffer.hpp"
#include "Rendering/Passes/Lighting.hpp"

#include <random>

void BuildScene(Scene& scene) {
    auto camera = scene.getCamera();
    Vec3 cameraPosition = Vec3(2.5, 2.5, 2.5);
    camera->getAttachNode()->getTransform().setTranslate(cameraPosition);
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
}

void CreateMaterials(Scene& scene) {
    MaterialConfig materialConfig{};
    Material* material = new Material{ "pbr", materialConfig };
    material->setShader({"deferredlighting.vert", "deferredlighting.frag"});
    material->setProperty("gPosition", Property{ 0, PropertyType::TEXTURE });
    material->setProperty("gAlbedo", Property{ 1, PropertyType::TEXTURE });
    material->setProperty("gNormal", Property{ 2, PropertyType::TEXTURE });
    material->setProperty("gARM", Property{ 3, PropertyType::TEXTURE });
    GraphicsPass* gBufferPass = new GBufferPass{ &scene };
    GraphicsPass* lightingPass = new LightingPass{ &scene };
    material->addPass(0, gBufferPass);
    material->addPass(1, lightingPass);

    scene.addMaterial(1000, material);
}