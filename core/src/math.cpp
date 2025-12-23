#include "ducklib/core/math.h"

#include <cassert>

namespace ducklib {
Vector3 Vector3::operator-(Vector3 o) const {
    return Vector3 { x - o.x, y - o.y, z - o.z };
}

Vector3 Vector3::operator+(Vector3 o) const {
    return Vector3 { x + o.x, y + o.y, z + o.z };
}

float Vector3::length() const {
    return std::sqrt(x*x + y*y + z*z);
}

float Vector3::length_sq() const {
    return x*x + y*y + z*z;
}

Vector3& Vector3::normalize() {
    auto len = length();
    x /= len;
    y /= len;
    z /= len;
    return *this;
}

Vector3 Vector3::normalized() const {
    auto len = length();
    return Vector3 { x/len, y/len, z/len };
}

float dot(Vector3 v1, Vector3 v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vector3 cross(Vector3 v1, Vector3 v2) {
    return Vector3 {
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x };
}

Matrix4::Matrix4() {
    *this = identity;
}

Matrix4::Matrix4(
    float m00, float m01, float m02, float m03,
    float m10, float m11, float m12, float m13,
    float m20, float m21, float m22, float m23,
    float m30, float m31, float m32, float m33) {
    m(0, 0) = m00; m(0, 1) = m01; m(0, 2) = m02; m(0, 3) = m03;
    m(1, 0) = m10; m(1, 1) = m11; m(1, 2) = m12; m(1, 3) = m13;
    m(2, 0) = m20; m(2, 1) = m21; m(2, 2) = m22; m(2, 3) = m23;
    m(3, 0) = m30; m(3, 1) = m31; m(3, 2) = m32; m(3, 3) = m33;
}

Vector3 Matrix4::operator*(Vector3 o) const {
    return Vector3{
        m(0, 0) * o.x + m(0, 1) * o.y + m(0, 2) * o.z,
        m(1, 0) * o.x + m(1, 1) * o.y + m(1, 2) * o.z,
        m(2, 0) * o.x + m(2, 1) * o.y + m(2, 2) * o.z
    };
}

Matrix4 Matrix4::operator*(const Matrix4& o) const {
    return Matrix4 {
        m(0, 0) * o(0, 0) + m(0, 1) * o(1, 0) + m(0, 2) * o(2, 0) + m(0, 3) * o(3, 0),
        m(0, 0) * o(0, 1) + m(0, 1) * o(1, 1) + m(0, 2) * o(2, 1) + m(0, 3) * o(3, 1),
        m(0, 0) * o(0, 2) + m(0, 1) * o(1, 2) + m(0, 2) * o(2, 2) + m(0, 3) * o(3, 2),
        m(0, 0) * o(0, 3) + m(0, 1) * o(1, 3) + m(0, 2) * o(2, 3) + m(0, 3) * o(3, 3),
        m(1, 0) * o(0, 0) + m(1, 1) * o(1, 0) + m(1, 2) * o(2, 0) + m(1, 3) * o(3, 0),
        m(1, 0) * o(0, 1) + m(1, 1) * o(1, 1) + m(1, 2) * o(2, 1) + m(1, 3) * o(3, 1),
        m(1, 0) * o(0, 2) + m(1, 1) * o(1, 2) + m(1, 2) * o(2, 2) + m(1, 3) * o(3, 2),
        m(1, 0) * o(0, 3) + m(1, 1) * o(1, 3) + m(1, 2) * o(2, 3) + m(1, 3) * o(3, 3),
        m(2, 0) * o(0, 0) + m(2, 1) * o(1, 0) + m(2, 2) * o(2, 0) + m(2, 3) * o(3, 0),
        m(2, 0) * o(0, 1) + m(2, 1) * o(1, 1) + m(2, 2) * o(2, 1) + m(2, 3) * o(3, 1),
        m(2, 0) * o(0, 2) + m(2, 1) * o(1, 2) + m(2, 2) * o(2, 2) + m(2, 3) * o(3, 2),
        m(2, 0) * o(0, 3) + m(2, 1) * o(1, 3) + m(2, 2) * o(2, 3) + m(2, 3) * o(3, 3),
        m(3, 0) * o(0, 0) + m(3, 1) * o(1, 0) + m(3, 2) * o(2, 0) + m(3, 3) * o(3, 0),
        m(3, 0) * o(0, 1) + m(3, 1) * o(1, 1) + m(3, 2) * o(2, 1) + m(3, 3) * o(3, 1),
        m(3, 0) * o(0, 2) + m(3, 1) * o(1, 2) + m(3, 2) * o(2, 2) + m(3, 3) * o(3, 2),
        m(3, 0) * o(0, 3) + m(3, 1) * o(1, 3) + m(3, 2) * o(2, 3) + m(3, 3) * o(3, 3)
        };
}

Matrix4& Matrix4::transpose() {
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 4; j++) {
            auto temp = m(i, j);
            m(i, j) = m(j, i);
            m(j, i) = temp;
        }
    }

    return *this;
}

Matrix4 Matrix4::transposed() const {
    return Matrix4 {
        m(0, 0), m(1, 0), m(2, 0), m(3, 0),
        m(0, 1), m(1, 1), m(2, 1), m(3, 1),
        m(0, 2), m(1, 2), m(2, 2), m(3, 2),
        m(0, 3), m(1, 3), m(2, 3), m(3, 3)
    };
}

Matrix4 Matrix4::identity {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f};

Matrix4 Matrix4::translation(float x, float y, float z) {
    return Matrix4{
        1.0f, 0.0f, 0.0f, x,
        0.0f, 1.0f, 0.0f, y,
        0.0f, 0.0f, 1.0f, z,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

Matrix4 Matrix4::rotation_x(float radians) {
    auto c = std::cos(radians);
    auto s = std::sin(radians);

    return Matrix4{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, c, s, 0.0f,
        0.0f, -s, c, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

Matrix4 Matrix4::rotation_y(float radians) {
    auto c = std::cos(radians);
    auto s = std::sin(radians);

    return Matrix4{
        c, 0.0f, -s, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        s, 0.0f, c, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

Matrix4 Matrix4::rotation_z(float radians) {
    auto c = std::cos(radians);
    auto s = std::sin(radians);

    return Matrix4{
        c, -s, 0.0f, 0.0f,
        s, c, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

Matrix4 Matrix4::look_at_lh(Vector3 eye, Vector3 target, Vector3 up) {
    auto dir = (target - eye).normalize();
    auto right = cross(up, dir).normalize();
    auto real_up = cross(dir, right);

    return Matrix4 {
        right.x, real_up.x, dir.x, 0.0f,
        right.y, real_up.y, dir.y, 0.0f,
        right.z, real_up.z, dir.z, 0.0f,
        -dot(right, eye), -dot(real_up, eye), -dot(dir, eye), 1.0f };
}

Matrix4 Matrix4::perspective_lh(float fov_rad, float aspect, float near, float far) {
    assert(fov_rad > 0.0f && fov_rad < std::numbers::pi);
    assert(aspect > 0.0f);
    assert(near > 0.0f && near < far);
    
    auto t = std::tan(fov_rad / 2.0f);
    auto fmn = far - near;

    return Matrix4 {
        1.0f / (aspect * t), 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f / t, 0.0f, 0.0f,
        0.0f, 0.0f, far / fmn, 1.0f,
        0.0f, 0.0f, -near * far / fmn, 0.0f };
}
}
