#pragma once

#include <vector>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/mat2x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/matrix.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

namespace math
{
    constexpr float pi = glm::pi<float>();
    constexpr float pi_2 = pi * 2.0f;
    constexpr float pi_half = pi * 0.5f;

    using Mat4 = glm::mat4x4;
    using Mat3 = glm::mat3x3;
    using Mat2 = glm::mat2x2;

    using Vec4 = glm::vec4;
    using Vec3 = glm::vec3;
    using Vec2 = glm::vec2;

    using Vec4i = glm::ivec4;
    using Vec3i = glm::ivec3;
    using Vec2i = glm::ivec2;

    constexpr Mat4 Identity(1);
    extern Vec3 X_Axis;
    extern Vec3 Y_Axis;
    extern Vec3 Z_Axis;

    extern Mat4 TempMat4;
    extern Vec4 TempVec4;
    extern Vec3 TempVec2;
    extern Vec2 TempVec3;

    inline Mat4 translate(const Vec3& v) { return glm::translate(Mat4(1.0f), v); }

    inline Mat4 scale(const Vec3& v) { return glm::scale(Mat4(1.0f), v); }

    inline Mat4 rotate(float angle, const Vec3& v) { return glm::rotate(Mat4(1.0f), angle, v); }
    inline Mat4 rotateDegree(float angle, const Vec3& v) { return glm::rotate(Mat4(1.0f), glm::radians(angle), v); }

    inline Mat4 rotateX(float angle) { return glm::eulerAngleX(angle); }
    inline Mat4 rotateY(float angle) { return glm::eulerAngleY(angle); }
    inline Mat4 rotateZ(float angle) { return glm::eulerAngleZ(angle); }
    /// rotateX(x) * rotateY(y) * rotateZ(z);
    inline Mat4 rotateXYZ(const Vec3& v) { return glm::eulerAngleXYZ(v.x, v.y, v.z); }
    /// rotateX(z) * rotateY(y) * rotateZ(x);
    inline Mat4 rotateZYX(const Vec3& v) { return glm::eulerAngleZYX(v.z, v.y, v.x); }

    inline Mat4 perspective(float fovy, float aspect, float zNear, float zFar)
    {
        return glm::perspective<float>(fovy, aspect, zNear, zFar);
    }

    inline Mat4 perspectiveYDown(float fovy, float aspect, float zNear, float zFar)
    {
        auto mat =  glm::perspective<float>(fovy, aspect, zNear, zFar);
        mat[1][1] *= -1;
        return mat;
    }

    inline Mat4 orgho(float left, float right, float bottom, float top, float zNear, float zFar)
    {
        return glm::ortho<float>(left, right, bottom, top, zNear, zFar);
    }

    inline Mat4 orghoYDown(float left, float right, float bottom, float top, float zNear, float zFar)
    {
        auto mat =  glm::ortho<float>(left, right, bottom, top, zNear, zFar);
        mat[1][1] *= -1;
        mat[3][1] *= -1;
        return mat;
    }

    inline Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) { return glm::lookAt(eye, center, up); }

    inline Mat4 lookTo(const Vec3& eye, const Vec3& zDir, const Vec3& up) { return glm::lookAt(eye, eye - zDir, up); }


    inline bool equal(float a, float b)
    {
        return std::abs(a - b) <= FLT_EPSILON;
    }

    inline bool equal(double a, double b)
    {
        return std::fabs(a - b) < DBL_EPSILON;
    }

    inline bool equal(const Mat4& mat1, const Mat4& mat2)
    {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                if (!equal(mat1[i][j], mat2[i][j]))
                {
                    return false;
                }
            }
        }
        return true;
    }

    inline bool equal(const Mat3& mat1, const Mat3& mat2)
    {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (!equal(mat1[i][j], mat2[i][j]))
                {
                    return false;
                }
            }
        }
        return true;
    }

    void dump(const Mat4&, const char* name = "");
    void dump(const Mat3&, const char* name = "");
    void dump(const Vec2&, const char* name = "");
    void dump(const Vec3&, const char* name = "");
    void dump(const Vec4&, const char* name = "");

    // 线性规划，用于判断当前点p 在p1, p2直线的哪一边.
    // Straight line equation: (x2 - x1) * (y - y1) - (y2 - y1) * (x - x1) = 0
    inline float lineEquation(const math::Vec2& p, const math::Vec2& p1, const math::Vec2& p2)
    {
        return  (p1.x - p2.x) * (p.y - p2.y) - (p1.y - p2.y) * (p.x - p2.x);
    }

    inline bool isInLine(const math::Vec2& p, const math::Vec2& v1, const math::Vec2& v2)
    {
        return math::equal(lineEquation(p, v1, v2), 0.0f);
    }

    inline float cross(const math::Vec2 &r, const math::Vec2 &l)
    {
        return r.x * l.y - r.y * l.x;
    }

    inline float angle(const math::Vec2 &v1, const math::Vec2 &v2)
    {
        float dz = v1.x * v2.y - v1.y * v2.x;
        return atan2(std::fabs(dz), glm::dot(v1, v2));
    }

    bool inPolygon(const Vec2& point, const std::vector<Vec2>& polygon);
    bool inPolygon(const Vec2& point, const std::vector<Vec3>& polygon);
}// namespace math