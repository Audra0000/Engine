#include "Time.h"
#include <SDL3/SDL.h>

Time::Time() : Module(), deltaTime(0.0f), totalTime(0.0f), lastFrame(0.0f)
{
}

Time::~Time()
{
}

bool Time::Start()
{
	lastFrame = SDL_GetTicks() / 1000.0f;
	return true;
}

bool Time::PreUpdate()
{
	float currentFrame = SDL_GetTicks() / 1000.0f;
	deltaTime = currentFrame - lastFrame;
	totalTime = currentFrame;
	lastFrame = currentFrame;
	return true;
}