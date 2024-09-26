//
//  Window.hpp
//  ToyEngine
//
//  Created by Li Han on 9/25/24.
//

#pragma once

#include "../Core/GPUFramework/Vulkan/VkCommon.hpp"

#include <GLFW/glfw3.h>
#include <string>

class Window{
public:
    Window(std::string name = "", int width = 1080, int height = 720);
    ~Window();
    
    bool shouldClose();
    
    void pollEvents();
    
    void waitEvents();
    
    void getFramebufferSize(int* width, int* height);
    
    VkResult createWindowSurface(VkInstance);
    
    VkSurfaceKHR getSurface();
    
protected:
    GLFWwindow* window;
    std::string name;
    int width;
    int height;
    
    VkSurfaceKHR surface;
};
