#include "InputManager.hpp"

#include <iostream>

InputManager::InputManager()
{
    SDL_ShowCursor(SDL_DISABLE);
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            controller = SDL_GameControllerOpen(i);
            if (controller) {
                std::cout << "Game controller connected: " << SDL_GameControllerName(controller) << std::endl;
                break;
            }
        }
    }
}

InputManager::~InputManager()
{
    if (controller) {
        SDL_GameControllerClose(controller);
    }
}

float InputManager::getAxis(SDL_GameControllerAxis axis) const {
    if (!controller) return 0.0f;
    return SDL_GameControllerGetAxis(controller, axis) / 32767.0f;
}
