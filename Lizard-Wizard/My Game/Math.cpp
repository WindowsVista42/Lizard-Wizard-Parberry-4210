#include "Math.h"

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

Vec3 JitterVector3(Vec3 input, f32 negativeAccuracy, f32 range) {
    return Vec3(
        input.x + ((negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.),
        input.y + ((negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.),
        input.z + ((negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.)
    );
}
