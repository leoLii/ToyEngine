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

VkResult Window::createWindowSurface(VkInstance instance){
    return glfwCreateWindowSurface(instance, window, nullptr, &surface);
}

VkSurfaceKHR& Window::getSurface() {
    return this->surface;
}
