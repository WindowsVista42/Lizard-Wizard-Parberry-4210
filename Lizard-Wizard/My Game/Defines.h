#include <cstdint>
#include <Renderer3D.h>
#include <btBulletCollisionCommon.h>
#include "Random.h"
#include <Abort.h>

//NOTE(sean): if you read this and youre wondering why on earth i would do this i just like the type names better

typedef wchar_t wchar;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uintptr_t usize;

typedef bool b8;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef intptr_t isize;

typedef float f32;
typedef double f64;

typedef VertexPositionNormalTexture VertexPNT;
typedef VertexPositionColor VertexPC;

#define every(name, count) (usize (name) = 0; (name) < (count); (name) += 1)

#define BT_SHAPE_TYPE_BOX 0
#define BT_SHAPE_TYPE_CAPSULE 10

/// These are for Cache size configurations
#define LIGHTS_CACHE_SIZE 254
#define PROJECTILE_CACHE_SIZE 64
#define NPC_CACHE_SIZE 24

/// Math
#define PI 3.14159265359f

/// Formatted abort when (lhs == rhs)
#define ABORT_EQ_FORMAT(lhs, rhs, format, ...) \
if((lhs) == (rhs)) { \
    ABORT_INNER(lhs, rhs, format, __FILE__, __LINE__, __VA_ARGS__); \
}

/// Formatted abort when (lhs != rhs)
#define ABORT_NE_FORMAT(lhs, rhs, format, ...) \
if((lhs) != (rhs)) { \
    ABORT_INNER(lhs, rhs, format, __FILE__, __LINE__, __VA_ARGS__); \
}

#define ABORT_INNER(lhs, rhs, format, file, line, ...) \
char message[1024]; \
sprintf(message, format, __VA_ARGS__); \
char f[1024]; \
sprintf(f, "%s:%i aborted at message : ", file, line); \
char b[1024]; \
sprintf(b, "%s\"%s\"", f, message); \
ABORT(b); \

#ifndef DEFINES_H
#define DEFINES_H

struct Mat4x4 : XMMATRIX {
    Mat4x4() {}
    Mat4x4(const XMMATRIX& other) { *this = *(Mat4x4*)&other; }

    operator XMMATRIX() const { return *(XMMATRIX*)this; }
};

struct Vec3 : Vector3 {
    Vec3() {}
    Vec3(const f32 splat) { this->x = splat; this->y = splat; this->z = splat; }
    Vec3(const f32 x, const f32 y, const f32 z) { this->x = x; this->y = y; this->z = z; }

    Vec3(const Vector3& other) { *this = *(Vec3*)&other; }
    Vec3(const btVector3& other) { *this = *(Vec3*)&other; }
    Vec3(const XMVECTOR& other) { *this = *(Vec3*)&other; }
    Vec3(const XMVECTORF32& other) { *this = *(Vec3*)&other; }
    Vec3(const XMFLOAT3& other) { *this = *(Vec3*)&other; }

    operator Vector3() const { return *(Vector3*)this; }
    operator btVector3() const { return *(btVector3*)this; }
    operator XMVECTOR() const { return *(XMVECTOR*)this; }
    operator XMVECTORF32() const { return *(XMVECTORF32*)this; }
    operator XMFLOAT3() const { return *(XMFLOAT3*)this; }
};

struct Vec4 : Vector4 {
    Vec4() {}
    Vec4(const f32 splat) { this->x = splat; this->y = splat; this->z = splat; this->w = splat; }
    Vec4(const f32 x, const f32 y, const f32 z, const f32 w) { this->x = x; this->y = y; this->z = z; this->w = w; }

    Vec4(const Vector4& other) { *this = *(Vec4*)&other; }
    Vec4(const btVector4& other) { *this = *(Vec4*)&other; } 
    Vec4(const XMVECTOR& other) { *this = *(Vec4*)&other; }
    Vec4(const XMVECTORF32& other) { *this = *(Vec4*)&other; }
    Vec4(const XMFLOAT4& other) { *this = *(Vec4*)&other; }

    operator Vector4() const { return *(Vector4*)this; }
    operator btVector4() const { return *(btVector4*)this; }
    operator XMVECTOR() const { return *(XMVECTOR*)this; }
    operator XMVECTORF32() const { return *(XMVECTORF32*)this; }
    operator XMFLOAT4() const { return *(XMFLOAT4*)this; }
    operator FLOAT*() const { return (FLOAT*)this; }
};

struct Quat : Quaternion {
    Quat() {}
    Quat(const f32 x, const f32 y, const f32 z, const f32 w) { this->x = x; this->y = y; this->z = z; this->w = w; }

    Quat(const Vector4& other) { *this = *(Quat*)&other; }
    Quat(const btVector4& other) { *this = *(Quat*)&other; } 
    Quat(const btQuaternion& other) { *this = *(Quat*)&other; } 
    Quat(const XMVECTOR& other) { *this = *(Quat*)&other; }
    Quat(const XMVECTORF32& other) { *this = *(Quat*)&other; }
    Quat(const XMFLOAT4& other) { *this = *(Quat*)&other; }

    operator Vector4() const { return *(Vector4*)this; }
    operator btVector4() const { return *(btVector4*)this; }
    operator btQuaternion() const { return *(btQuaternion*)this; }
    operator XMVECTOR() const { return *(XMVECTOR*)this; }
    operator XMVECTORF32() const { return *(XMVECTORF32*)this; }
    operator XMFLOAT4() const { return *(XMFLOAT4*)this; }
};

class GameRandom : public LRandom {
public:
    static GameRandom& Get() {
        static GameRandom instance;
        return instance;
    }

    static f32 Randf32() {
        return Get().randf();
    }

    static u32 Randu32() {
        return Get().randn();
    }

    static u32 Randu32(u32 min, u32 max) {
        return Get().randn(min, max);
    }
};

struct Light {
    Vec4 position;
    Vec4 color;
};

// SET YOUR DEFINES HERE

namespace SoundIndex { enum e : u32 {
    Clang, Grunt, Impact,
    Size  //MUST BE LAST
};}

namespace ModelIndex { enum e : u32 {
    Cube, Suzanne, Obelisk, Sentry, Quad,
    Staff, Orb,
    Count // keep this last
};}

namespace TextureIndex { enum e : u32 {
    Font, // keep this first
    Other, White,
    Count // keep this last
};}

namespace NPCBehavior { enum e : u32 { 
    MELEE, RANGED, TURRET, BOSS
};}

namespace NPCState { enum e : u32 {
    SLEEPING, WANDER, MOVING, PATHFINDING, ATTACKING, SEARCHING, ANIMATING
};}

namespace RoomTag {
    enum tag : u32 {
        UNFILLED, CORRIDOR, NSHALL, EWHALL, NORMAL, BOSS, SPAWN, NWL, SWL, NEL, SEL
    };
}

namespace ExitTag {
    enum exit : u32 {
        NORTHEX, SOUTHEX, EASTEX, WESTEX
    };
}

#endif
