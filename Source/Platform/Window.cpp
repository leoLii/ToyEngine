//
//  Window.cpp
//  ToyEngine
//
//  Created by Li Han on 9/25/24.
//

#include "Window.hpp"

Window::Window(std::string name, int width, int height)
:name(name), width(width), height(height){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    this->window = glfwCreateWindow(width, height, "Vulkan window", nullptr, nullptr);
}

Window::~Window(){
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Window::shouldClose(){
    return glfwWindowShouldClose(this->window);
}

void Window::pollEvents(){
    glfwPollEvents();
}

void Window::waitEvents(){
    glfwWaitEvents();
}

void Window::getFramebufferSize(int* width, int* height){
    glfwGetFramebufferSize(this->window, width, height);
}

std::vector<const char*> Window::requireWindowExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    return std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
}

GLFWwindow* Window::getHandle()
{
    return window;
}
