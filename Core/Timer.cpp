#include "Timer.h"
#include <SDL3/SDL.h>

Timer::Timer()
{
    previousTime = SDL_GetTicks() / 1000.0;
}

void Timer::Update()
{
    double currentTime = SDL_GetTicks() / 1000.0;

    deltaTime = currentTime - previousTime;
    previousTime = currentTime;

    frameTime = static_cast<float>(deltaTime * 1000.0);

    if (deltaTime > 0.0)
    {
        fps = static_cast<float>(1.0 / deltaTime);
    }
}

float Timer::GetDeltaTime() const
{
    return static_cast<float>(deltaTime);
}

float Timer::GetFPS() const
{
    return fps;
}

float Timer::GetFrameTime() const
{
    return frameTime;
}