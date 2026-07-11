#include "Camera.h"

Matrix4 Camera::GetViewMatrix() const
{
    return
        Matrix4::RotationX(-rotation.x) *
        Matrix4::RotationY(-rotation.y) *
        Matrix4::RotationZ(-rotation.z) *
        Matrix4::Translation(-position);
}   