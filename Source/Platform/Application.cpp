#include "Application.hpp"



#include "Scene/Scene.hpp"
#include "Scene/Mesh.hpp"
#include "Scene/Components/Camera.hpp"

#include "Core/GPUFramework/Vulkan/TextureVulkan.hpp"
#include "Core/TextureManager.hpp"
#include "Core/Passes/GBuffer.hpp"
#include "Core/Passes/Lighting.hpp"
#include "Core/Passes/Taa.hpp"
#include "Core/Passes/FrustumCull.hpp"

#include "Rendering/RenderContext.hpp"

#include <cstddef>
#include <functional>
#include <thread>
#include <future>

void Application::init(ApplicationConfig& config, Scene* scene)
{
	window = new Window{ config.name, config.width, config.height };
	auto windowExtensions = Window::requireWindowExtensions();
	config.extensions.insert(config.extensions.end(), windowExtensions.begin(), windowExtensions.end());
	
	GPUContext::GetSingleton().init(config.name, config.layers, config.extensions, window);

	std::vector<const char*> texturePath = {
		"C:/Users/lihan/Desktop/workspace/ToyEngine/Resource/cat/textures/diffuse.ktx2",
		"C:/Users/lihan/Desktop/workspace/ToyEngine/Resource/cat/textures/normal.ktx2",
		"C:/Users/lihan/Desktop/workspace/ToyEngine/Resource/cat/textures/metal.ktx2",
		"C:/Users/lihan/Desktop/workspace/ToyEngine/Resource/cat/textures/roughness.ktx2"
	};
	
	TextureManager::GetSingleton().createTextureReference(std::move(texturePath));
	
	this->scene = scene;

	RenderContext::GetSingleton().prepare(scene);
}

void Application::run()
{
	while (!window->shouldClose()) {

		++frameIndex;

		scene->update(frameIndex);

		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
		deltaTime = std::chrono::duration<double, std::milli>(now - lastFrameTime).count() / 1000.0;
		lastFrameTime = now;

		window->pollEvents();

		RenderContext::GetSingleton().render(frameIndex);
	}
}

void Application::close()
{
	delete window;
	RenderContext::GetSingleton().clear();
}