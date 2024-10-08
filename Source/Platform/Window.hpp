//
//  Window.hpp
//  ToyEngine
//
//  Created by Li Han on 9/25/24.
//

#pragma once

#include "Core/GPUFramework/Vulkan/VkCommon.hpp"

#include <GLFW/glfw3.h>
#include <string>
#include <vector>

class Window{
public:
    Window(std::string name = "", int width = 1920, int height = 1080);
    ~Window();
    
    bool shouldClose();
    
    void pollEvents();
    
    void waitEvents();
    
    void getFramebufferSize(int* width, int* height);
    
    void createWindowSurface(vk::Instance);
    
    const vk::SurfaceKHR& getSurface() const;

    static std::vector<const char*> requireWindowExtensions();
    
protected:
    GLFWwindow* window;
    std::string name;
    size_t width;
    size_t height;
    
    vk::SurfaceKHR surface;
};
