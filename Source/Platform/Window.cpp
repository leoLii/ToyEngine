//
//  Window.cpp
//  ToyEngine
//
//  Created by Li Han on 9/25/24.
//

#include "Window.hpp"

Window::Window(std::string name, size_t width, size_t height)
:name(name), width(width), height(height){
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
    SDL_SetWindowGrab(window, SDL_TRUE);
}

Window::~Window(){
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool Window::shouldClose() {
    return closeSemaphore;
}

void Window::pollEvents(){
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type)
        {
        case SDL_QUIT:
            closeSemaphore = true;
            break;
        
        default:
            break;
        }
    }
}

void Window::waitEvents(){
    SDL_WaitEvent(nullptr);
}

void Window::getFramebufferSize(int* width, int* height){
    SDL_GetWindowSize(window, width, height);
}

std::vector<const char*> Window::requireWindowExtensions()
{
    uint32_t count = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
    std::vector<const char*> extensions(count);
    SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data());
    return extensions;
}

SDL_Window* Window::getHandle()
{
    return window;
}
