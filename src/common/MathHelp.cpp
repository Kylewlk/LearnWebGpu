#include "MathHelp.h"
#include <iostream>

namespace math
{
    Vec3 X_Axis{1, 0, 0};
    Vec3 Y_Axis{0, 1, 0};
    Vec3 Z_Axis{0, 0, 1};

    Mat4 TempMat4{1.0f};;
    Vec4 TempVec4{0.0f};
    Vec3 TempVec2{0.0f};
    Vec2 TempVec3{0.0f};

    inline float printNumber(float f)
    {
        if (std::abs(f) < 1e-6 )
        {
            return 0;
        }
        return f;
    }

    void dump(const Mat4& m, const char* name /*= ""*/)
    {
        std::cout<< name << " Mat4:\n ";
        for (int i = 0; i < 4; ++i) {
            std::cout << "    ";
            for (int j = 0; j < 4; ++j) {
                std::cout << printNumber(m[j][i]) << ", ";
            }
            std::cout<<'\n';
        }
    }

    void dump(const Mat3& m, const char* name /*= ""*/)
    {
        std::cout<< name << " Mat3:\n ";
        for (int i = 0; i < 3; ++i) {
            std::cout << "    ";
            for (int j = 0; j < 3; ++j) {
                std::cout << printNumber(m[j][i]) << ", ";
            }
            std::cout<<'\n';
        }
    }

    void dump(const Vec2& v, const char* name /*= ""*/)
    {
        std::cout<< name << " Vec2("<<printNumber(v.x) << ", "<<printNumber(v.y) << ")\n";
    }

    void dump(const Vec3& v, const char* name /*= ""*/)
    {
        std::cout<< name << " Vec3("<<printNumber(v.x) << ", "<<printNumber(v.y) << ", " <<printNumber(v.z) << ")\n";
    }

    void dump(const Vec4& v, const char* name /*= ""*/)
    {
        std::cout<< name << " Vec4("<<printNumber(v.x) << ", "<<printNumber(v.y) << ", "
                  <<printNumber(v.z) << ", "<<printNumber(v.w) << ")\n";
    }

    bool inPolygon(const Vec2& point, const std::vector<Vec2>& polygon)
    {
        // 水平向右发射一条射线，如果与多边形交点是奇数个，则点在多边形内.
        auto count = polygon.size();
        bool c = false;
        for (int i = 0, j = count - 1; i < count; j = i, ++i)
        {
            if ((polygon[i].y > point.y) != (polygon[j].y > point.y))
            {
                if((polygon[j].y > polygon[i].y && lineEquation(polygon[i], polygon[j], point) > 0)
                    || (polygon[j].y < polygon[i].y && lineEquation(polygon[i], polygon[j], point) < 0))
                {
                    c = !c;
                }
            }
        }
        return c;
    }

    bool inPolygon(const Vec2& point, const std::vector<Vec3>& polygon)
    {
        // 水平向右发射一条射线，如果与多边形交点是奇数个，则点在多边形内.
        auto count = polygon.size();
        bool c = false;
        for (int i = 0, j = count - 1; i < count; j = i, ++i)
        {
            if ((polygon[i].y > point.y) != (polygon[j].y > point.y))
            {
                if((polygon[j].y > polygon[i].y && lineEquation(polygon[i], polygon[j], point) > 0)
                    || (polygon[j].y < polygon[i].y && lineEquation(polygon[i], polygon[j], point) < 0))
                {
                    c = !c;
                }
            }
        }
        return c;
    }

}