#pragma once
#ifndef TJH_MATH_H
#define TJH_MATH_H

// TODO: test all the functions with various values!
// - dot
// - cross
// - lengthSquared
// - length
// - += vec +ve, -ve, zero
// - -= vec ...
// - *= vec ..
// - /= vec .
// - += float +ve, -ve, zero
// - -= float ...
// - *= float ..
// - /= float .
// - +, -, *, / vec
// - +, -, *, / float

// If this is a good idea, maybe I could make it a proper thing, test against GLM?
// MORE TYPES:
// - aabb2, aabb3
// - point2, point3
// - circle, sphere
// - matrix types!
//
// Other userfull things, see gb_math.h and HandmadeMath.h
// Best of all is possibly linalg.h
// - put in namespace
// - degrees to radians, radians to degrees
// - math constants
// - interpolation of various kinds
// - colour conversions RGB to HLS and HLS to RGB
// - SSE
// - watch for divide by zero error in sqrt normalizing
// - overload '<<' stream operator for easy printing

#include <cstdint>
#include <cmath>
#include <ostream>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

// TODO: make these better / check they are accurate
const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;
const float DEG_TO_RAD = 0.01745329252;
const float RAD_TO_DEG = 57.295779513;

struct vec3
{
    union {
        struct { float x; float y; float z; };
        struct { float r; float g; float b; };
        float e[3]; // TODO: overload the [] operator to make this unnecessary?
                    // Would that actually be an improvement?
    };

    vec3() : x(0), y(0), z(0) {}
    vec3( float x, float y, float z ) : x(x), y(y), z(z) {}

    inline float dot( const vec3& rhs )  const { return x*rhs.x + y*rhs.y + z*rhs.z; }
    inline vec3 cross( const vec3& rhs ) const { return vec3( y*rhs.z - z*rhs.y, z*rhs.x - x*rhs.z, x*rhs.y - y*rhs.x ); }
    inline vec3 normal() const {
        float len = length();
        if( len == 0.0f ) return vec3();
        else return vec3( x / len, y / len, z / len );
    }
    inline vec3& normalize() {
        float len = length();
        if( len != 0.0f ) { x /= len; y /= len; z /= len; }
        return *this;
    }
    inline float lengthSquared() const { return x*x + y*y + z*z; }
    inline float length()        const { return std::sqrt( lengthSquared() ); }
    inline float distanceSquared( const vec3& rhs ) const { return (*this - rhs).lengthSquared(); }
    inline float distance( const vec3& rhs )        const { return (*this - rhs).length(); }

    inline bool operator == ( const vec3& rhs ) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
    inline bool operator != ( const vec3& rhs ) const { return ! (*this == rhs) ; }

    inline vec3& operator += ( const vec3& rhs ) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
    inline vec3& operator -= ( const vec3& rhs ) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
    inline vec3& operator *= ( const vec3& rhs ) { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
    inline vec3& operator /= ( const vec3& rhs ) { x /= rhs.x; y /= rhs.y; z /= rhs.z; return *this; }

    inline vec3& operator += ( float f ) { x += f; y += f; z += f; return *this; }
    inline vec3& operator -= ( float f ) { x -= f; y -= f; z -= f; return *this; }
    inline vec3& operator *= ( float f ) { x *= f; y *= f; z *= f; return *this; }
    inline vec3& operator /= ( float f ) { x /= f; y /= f; z /= f; return *this; }

    inline vec3 operator + ( const vec3& rhs ) const { return vec3( x+rhs.x, y+rhs.y, z+rhs.z ); }
    inline vec3 operator - ( const vec3& rhs ) const { return vec3( x-rhs.x, y-rhs.y, z-rhs.z ); }
    inline vec3 operator * ( const vec3& rhs ) const { return vec3( x*rhs.x, y*rhs.y, z*rhs.z ); }
    inline vec3 operator / ( const vec3& rhs ) const { return vec3( x/rhs.x, y/rhs.y, z/rhs.z ); }

    inline vec3 operator + ( float f ) const { return vec3( x+f, y+f, z+f ); }
    inline vec3 operator - ( float f ) const { return vec3( x-f, y-f, z-f ); }
    inline vec3 operator * ( float f ) const { return vec3( x*f, y*f, z*f ); }
    inline vec3 operator / ( float f ) const { return vec3( x/f, y/f, z/f ); }

};

std::ostream& operator << ( std::ostream& os, const vec3& v ) { os << "(" << v.x << ", " << v.y << ", " << v.z << ")"; return os; }

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

#endif
