#include "Matrix4.h"

#include <cstring>
#include <cmath>

Matrix4::Matrix4()
{
    std::memset(m, 0, sizeof(m));
}

Matrix4 Matrix4::Identity()
{
    Matrix4 result;

    result.m[0][0] = 1.0f;
    result.m[1][1] = 1.0f;
    result.m[2][2] = 1.0f;
    result.m[3][3] = 1.0f;

    return result;
}

Matrix4 Matrix4::Translation(const Vector3 &t)
{
    Matrix4 result = Identity();

    result.m[0][3] = t.x;
    result.m[1][3] = t.y;
    result.m[2][3] = t.z;

    return result;
}

Matrix4 Matrix4::Scale(const Vector3 &s)
{
    Matrix4 result = Identity();

    result.m[0][0] = s.x;
    result.m[1][1] = s.y;
    result.m[2][2] = s.z;

    return result;
}

Matrix4 Matrix4::RotationX(float radians)
{
    Matrix4 result = Identity();

    float c = std::cos(radians);
    float s = std::sin(radians);

    result.m[1][1] = c;
    result.m[1][2] = -s;

    result.m[2][1] = s;
    result.m[2][2] = c;

    return result;
}

Matrix4 Matrix4::RotationY(float radians)
{
    Matrix4 result = Identity();

    float c = std::cos(radians);
    float s = std::sin(radians);

    result.m[0][0] = c;
    result.m[0][2] = s;

    result.m[2][0] = -s;
    result.m[2][2] = c;

    return result;
}

Matrix4 Matrix4::RotationZ(float radians)
{
    Matrix4 result = Identity();

    float c = std::cos(radians);
    float s = std::sin(radians);

    result.m[0][0] = c;
    result.m[0][1] = -s;

    result.m[1][0] = s;
    result.m[1][1] = c;

    return result;
}

Matrix4 Matrix4::operator*(const Matrix4 &other) const
{
    Matrix4 result;

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            result.m[row][col] = 0.0f;

            for (int k = 0; k < 4; k++)
            {
                result.m[row][col] +=
                    m[row][k] * other.m[k][col];
            }
        }
    }

    return result;
}

Vector3 Matrix4::MultiplyPoint(const Vector3 &p) const
{
    return {
        m[0][0] * p.x + m[0][1] * p.y + m[0][2] * p.z + m[0][3],

        m[1][0] * p.x + m[1][1] * p.y + m[1][2] * p.z + m[1][3],

        m[2][0] * p.x + m[2][1] * p.y + m[2][2] * p.z + m[2][3]};
}

Vector3 Matrix4::MultiplyVector(const Vector3 &v) const
{
    return {
        m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,

        m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,

        m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z};
}

Matrix4 Matrix4::Perspective(float fov, float aspect, float nearPlane, float farPlane)
{
    Matrix4 result{};

    float f = 1.0f / std::tan(fov * 0.5f);

    result.m[0][0] = f / aspect;
    result.m[1][1] = f;

    result.m[2][2] = (farPlane + nearPlane) / (nearPlane - farPlane);
    result.m[2][3] = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);

    result.m[3][2] = 1.0f;

    return result;
}

Vector4 Matrix4::MultiplyPoint4(const Vector3 &v) const
{
    return {
        m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3],
        m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3],
        m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3],
        m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3]};
}