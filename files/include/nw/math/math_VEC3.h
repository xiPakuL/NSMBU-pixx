#ifndef MATH_VEC3_H_
#define MATH_VEC3_H_

#include <math.h>

namespace nw { namespace math {

struct MTX34;

struct VEC3
{
    static const VEC3& Zero()
    {
        static const VEC3 zero = (VEC3){ 0.0f, 0.0f, 0.0f };
        return zero;
    }

    f32 MagnitudeSquare() const
    {
        return x*x + y*y + z*z;
    }

    f32 Magnitude() const
    {
        return sqrtf(MagnitudeSquare());
    }

    void Normalize()
    {
        f32 inv_mag = 1.0f / Magnitude(); // No division-by-zero check...
        x *= inv_mag;
        y *= inv_mag;
        z *= inv_mag;
    }

    static inline VEC3* CrossProduct(VEC3* dst, const VEC3* a, const VEC3* b);
    static inline f32 DotProduct(const VEC3* a, const VEC3* b);

    static inline VEC3* MultMTX(VEC3* dst, const VEC3* a, const MTX34* b);

    f32 x;
    f32 y;
    f32 z;
};

inline VEC3& operator+=(VEC3& lhs, const VEC3& rhs)
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    lhs.z += rhs.z;
    return lhs;
}

inline VEC3& operator-=(VEC3& lhs, const VEC3& rhs)
{
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    lhs.z -= rhs.z;
    return lhs;
}

inline VEC3& operator*=(VEC3& lhs, f32 a)
{
    lhs.x *= a;
    lhs.y *= a;
    lhs.z *= a;
    return lhs;
}

inline VEC3 operator+(const VEC3& lhs, const VEC3& rhs)
{
    VEC3 ret = lhs;
    return (ret += rhs);
}

inline VEC3 operator-(const VEC3& lhs, const VEC3& rhs)
{
    VEC3 ret = lhs;
    return (ret -= rhs);
}

inline VEC3 operator*(const VEC3& lhs, f32 a)
{
    VEC3 ret = lhs;
    return (ret *= a);
}

inline VEC3 operator*(f32 a, const VEC3& rhs)
{
    return (VEC3){ a * rhs.x, a * rhs.y, a * rhs.z };
}

VEC3* VEC3::CrossProduct(VEC3* dst, const VEC3* a, const VEC3* b)
{
    VEC3 tmp = {
        .x =  a->y * b->z - a->z * b->y,
        .y =  a->z * b->x - a->x * b->z,
        .z =  a->x * b->y - a->y * b->x,
    };

    *dst = tmp;
    return dst;
}

f32 VEC3::DotProduct(const VEC3* a, const VEC3* b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

} } // namespace nw::math

#endif // MATH_VEC3_H_
