#pragma once

class Timer
{
public:
    Timer();

    void Update();

    float GetDeltaTime() const;
    float GetFPS() const;
    float GetFrameTime() const;

private:
    double previousTime = 0.0;
    double deltaTime = 0.0;

    float fps = 0.0f;
    float frameTime = 0.0f;
};