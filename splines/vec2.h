#pragma once

#include <cmath>

struct vec2
{
    float x, y;

    vec2() : x(0), y(0) {}
    vec2( float x, float y ) : x(x), y(y) {}

    inline float dot( const vec2& rhs )             const { return x*rhs.x + y*rhs.y; }

    inline float lengthSquared()                    const { return x * x + y * y; }
    inline float length()                           const { return std::sqrt( lengthSquared() ); }
    inline float distanceSquared( const vec2& rhs ) const { return (*this - rhs).lengthSquared(); }
    inline float distance( const vec2& rhs )        const { return (*this - rhs).length(); }
    inline vec2  normalized()                       const { return *this / length(); }

    // Result is an angle in radians between -PI and PI
    inline float angle( const vec2& rhs ) { return std::atan2(x*rhs.y-y*rhs.x, x*rhs.x+y*rhs.y); }

    inline vec2& operator += ( const vec2& rhs ) { x += rhs.x; y += rhs.y; return *this; }
    inline vec2& operator -= ( const vec2& rhs ) { x -= rhs.x; y -= rhs.y; return *this; }

    inline vec2 operator + ( const vec2& rhs ) const { return vec2( x + rhs.x, y + rhs.y ); }
    inline vec2 operator - ( const vec2& rhs ) const { return vec2( x - rhs.x, y - rhs.y ); }

    inline vec2 operator * ( float f ) const { return vec2( x*f, y*f ); }
    inline vec2 operator / ( float f ) const { return vec2( x/f, y/f ); }
};

template<typename T>
inline T clamp(T val, T min, T max)
{
    return (val>max ? max : (val<min ? min : val));
}

template<typename T>
inline T lerp(T a, T b, float t)
{
    return a + (b - a) * t;
}

inline float distance(vec2 a, vec2 b)
{
    return a.distance(b);
}