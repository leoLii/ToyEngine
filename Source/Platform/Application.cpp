#include "Application.hpp"

#include "InputManager.hpp"

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
	auto windowExtensions = window->requireWindowExtensions();
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

	renderThread = std::thread(&Application::render, this);
}

void Application::run()
{
	while (!window->shouldClose()) {
		window->pollEvents();

		// Read joystick axis
		float moveX = InputManager::GetSingleton().getAxis(SDL_CONTROLLER_AXIS_LEFTX);
		float moveY = InputManager::GetSingleton().getAxis(SDL_CONTROLLER_AXIS_LEFTY);
		float rotateX = InputManager::GetSingleton().getAxis(SDL_CONTROLLER_AXIS_RIGHTX);
		float rotateY = InputManager::GetSingleton().getAxis(SDL_CONTROLLER_AXIS_RIGHTY);

		auto camera = scene->getCamera();
		// Movement
		if (fabs(moveX) > 0.1f || fabs(moveY) > 0.1f) {
			camera->move(Vec2(moveX, moveY), 0.1);
		}

		// Rotation
		if (fabs(rotateX) > 0.1f || fabs(rotateY) > 0.1f) {
			camera->rotate(Vec2(-rotateX, -rotateY), 0.4);
		}

		auto& frameData = RenderContext::GetSingleton().getFrameData(frameIndex.load());

		// 确保上一帧渲染完成
		while (!frameData.rendered) {
			std::this_thread::yield();  // 逻辑线程等待
		}

		beginFrame();

		frameData.readyForRender.store(true);
		frameData.rendered.store(false);

		scene->update(frameIndex);

		endFrame();
	}
}

void Application::close()
{
	renderThread.join();
	delete window;
	RenderContext::GetSingleton().clear();
}

void Application::beginFrame()
{
	++frameIndex;
	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	deltaTime = std::chrono::duration<double, std::milli>(now - lastFrameTime).count() / 1000.0;
	lastFrameTime = now;
}

void Application::endFrame()
{
	////Present here

}

void Application::render()
{
	while (!window->shouldClose()) {
		auto& frameData = RenderContext::GetSingleton().getFrameData(frameIndex);
		if (!frameData.readyForRender) {
			std::this_thread::yield();  // 渲染线程等待
			continue;
		}
		RenderContext::GetSingleton().render(frameIndex);
	}
}
