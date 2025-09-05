#ifndef MATH_H
#define MATH_H
#include <cmath>
#include <numbers>
#include <initializer_list>

namespace ducklib {
struct Matrix4;

inline float deg_to_rad(float degrees) {
    return degrees * (std::numbers::pi / 180.0f);
}

inline float rad_to_deg(float radians) {
    return radians * (180.0f / std::numbers::pi);
}

struct Vector3 {
    float x;
    float y;
    float z;

    Vector3 operator - (Vector3 o) const;
    Vector3 operator + (Vector3 o) const;

    float length() const;
    float length_sq() const;
    
    Vector3& normalize();
    Vector3 normalized() const;
};

float dot(Vector3 v1, Vector3 v2);
Vector3 cross(Vector3 v1, Vector3 v2);

struct Matrix4 {
private:
    float v[16];

public:
    Matrix4(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33);

    Vector3 operator * (Vector3 o) const;
    Matrix4 operator * (const Matrix4& o) const;

    inline float& operator()(int row, int column) {
        return v[row * 4 + column];
    }

    inline const float& operator()(int row, int column) const {
        return v[row * 4 + column];
    }

    static Matrix4 identity;
    static Matrix4 translation(float x, float y, float z);
    static Matrix4 rotation_x(float radians);
    static Matrix4 look_at(Vector3 eye, Vector3 target, Vector3 up);
    static Matrix4 perspective(float fov, float aspect, float near, float far);

private:
    float& m(int row, int column) {
        return (*this)(row, column);
    }

    const float& m(int row, int column) const {
        return (*this)(row, column);
    }
};
}
#endif
