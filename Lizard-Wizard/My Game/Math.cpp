#include "Math.h"
#include "Defines.h"

Mat4x4 MoveRotateScaleMatrix(Vec3 position, Quat rotation, Vec3 scale) {
    Mat4x4 world = XMMatrixRotationQuaternion(rotation);
    Mat4x4 scl = XMMatrixScaling(scale.x, scale.y, scale.z);
    world = XMMatrixMultiply(scl, world);
    world.r[3] = XMVectorSelect(world.r[3], position, g_XMSelect1110);

    return world;
}

Mat4x4 MoveScaleMatrix(Vec3 position, Vec3 scale) {
    Mat4x4 world = XMMatrixScaling(scale.x, scale.y, scale.z); // scale the world matrix
    world.r[3] = XMVectorSelect(world.r[3], position, g_XMSelect1110); // position the world matrix

    return world;
}

Vec3 JitterVec3(Vec3 input, f32 negativeAccuracy, f32 range) {
    f32 xnoise = range * (GameRandom::Randf32() - 0.5f) * 2.0f;
    f32 ynoise = range * (GameRandom::Randf32() - 0.5f) * 2.0f;
    f32 znoise = range * (GameRandom::Randf32() - 0.5f) * 2.0f;

    return Vec3(
        input.x + xnoise,
        input.y + ynoise,
        input.z + znoise
    );
}
