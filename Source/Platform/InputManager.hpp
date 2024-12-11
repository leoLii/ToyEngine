#pragma once

#include <SDL2/SDL.h>

class InputManager
{
public:
	static InputManager& GetSingleton() {
		static InputManager singleton{};
		return singleton;
	}

	float getAxis(SDL_GameControllerAxis) const;

protected:
	InputManager();
	~InputManager();

	SDL_GameController* controller;
};