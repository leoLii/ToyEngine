//
//  Window.hpp
//  ToyEngine
//
//  Created by Li Han on 9/25/24.
//

#pragma once

#define SDL_MAIN_HANDLED

#include "Core/GPUFramework/Vulkan/VkCommon.hpp"

#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

class Window{
public:
    Window(std::string name = "", size_t width = 1920, size_t height = 1080);
    ~Window();
    
    bool shouldClose();
    
    void pollEvents();
    
    void waitEvents();
    
    void getFramebufferSize(int* width, int* height);

    std::vector<const char*> requireWindowExtensions();

    SDL_Window* getHandle();
    
protected:
    SDL_Window* window{ nullptr };
    bool closeSemaphore = false;
    std::string name;
    size_t width;
    size_t height;
};
