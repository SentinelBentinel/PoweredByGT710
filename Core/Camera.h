#pragma once

#include "../Math/Vector3.h"
#include "../Math/Matrix4.h"

class Camera
{
public:
    Vector3 position = {0.0f,0.0f,0.0f};
    Vector3 rotation = {0.0f,0.0f,0.0f};

    Matrix4 GetViewMatrix() const;
    Vector3 Forward() const;
    Vector3 Right() const;
};