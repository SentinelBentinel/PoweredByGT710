#pragma once

#include "Vector3.h"
#include "Vector4.h"

struct Matrix4 {
    float m[4][4];

    Matrix4();

    static Matrix4 Identity();
    static Matrix4 Translation(const Vector3& translation);
    static Matrix4 Scale(const Vector3& scale);

    static Matrix4 RotationX(float radians);
    static Matrix4 RotationY(float radians);
    static Matrix4 RotationZ(float radians);

    Matrix4 operator*(const Matrix4& other) const;

    Vector3 MultiplyPoint(const Vector3& point) const;
    Vector3 MultiplyVector(const Vector3& vector) const;

    static Matrix4 Perspective(float fov, float aspect, float nearPlane, float farPlane);
    Vector4 MultiplyPoint4(const Vector3& point) const;
};