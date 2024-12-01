#pragma once

#include "Platform/Window.hpp"

#include <chrono>
#include <string>
#include <memory>
#include <vector>
#include <thread>

class Scene;
class Window;
class GBufferPass;
class LightingPass;
class TaaPass;
class FrustumCullPass;
class TextureManager;
class RenderContext;

struct ApplicationConfig
{
	std::string name;
	uint32_t width;
	uint32_t height;
	std::vector<const char*> layers;
	std::vector<const char*> extensions;
	uint32_t maxFrames = 2;
};

class Scene;

class Application {
public:
	Application() = default;
	~Application() = default;

	void init(ApplicationConfig& config, Scene* scene);

	void run();

	void close();

	void beginFrame();

	void endFrame();

	void loadImage();

	void render();

protected:
	ApplicationConfig config;

	Window* window;

	bool headless = false;

	bool resizable = false;

	bool shouldClose = false;

	bool vsyncOn = false;

	uint32_t fps = 0;

	uint64_t frameIndex = -1;

	float deltaTime = 0.0;

	std::chrono::time_point<std::chrono::system_clock> lastFrameTime;

	Scene* scene;

	std::thread renderThread;
};