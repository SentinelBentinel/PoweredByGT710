#include "Camera.h"

#include <cmath>

Matrix4 Camera::GetViewMatrix() const
{
    return Matrix4::RotationX(-rotation.x) *
           Matrix4::RotationY(-rotation.y) *
           Matrix4::RotationZ(-rotation.z) *
           Matrix4::Translation(-position);
}

Vector3 Camera::Forward() const
{
    float pitch = rotation.x;
    float yaw = rotation.y;

    Vector3 forward =
        {
            std::cos(pitch) * std::sin(yaw),
            -std::sin(pitch),
            std::cos(pitch) * std::cos(yaw)};

    return forward.Normalized();
}

Vector3 Camera::Right() const
{
    Vector3 forward = Forward();

    return Vector3::Cross({0.0f, 1.0f, 0.0f}, forward).Normalized();
}