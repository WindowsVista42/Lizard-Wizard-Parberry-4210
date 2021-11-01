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

/*
btQuaternion LookAt(Vec3 origin, Vec3 lookAt) {
    Vec3 forwardVector = XMVector3Normalize(origin - lookAt);
    Vec3 rotAxis = XMVector3Cross(Vec3(1.0f, 0, 0), forwardVector);
    rotAxis = Vec3(0.0f, 0.0f, 0.0f);
    f32 angle = acos(forwardVector.Dot(Vec3(1.0f, 0, 0)));

    btQuaternion q;
    q.setRotation(rotAxis, angle);

    q.setX(0.0f);
    q.setY(rotAxis.y);
    q.setZ(0.0f);
    q.setW(dot + 1);

    return q.normalize();
}
*/

btQuaternion CreateFromAxisAngle(Vec3 axis, f32 angle) {
    f32 halfAngle = angle * .5f;
    f32 s = (f32) sin(halfAngle);
    btQuaternion q;
    q.setX(axis.x * s);
    q.setY(axis.y * s);
    q.setZ(axis.z * s);
    q.setW((f32)cos(halfAngle));
    return q;
}

btQuaternion LookAt(Vec3 origin, Vec3 lookAt) {
    Vec3 forwardVector = XMVector3Normalize(lookAt - origin);
    Vec3 forward = Vec3(1.0f, 0, 0);
    f32 dot = forward.Dot(forwardVector);

    if (abs(dot - (-1.0f)) < 0.000001f) {
        return btQuaternion(1.0f, 0, 0, 3.1415926535897932f);
    }
    if (abs(dot - (1.0f)) < 0.000001f) {
        return btQuaternion(0,0,0,1.0f);
    }

    f32 angle = (f32)acos(dot);
    Vec3 rotAxis = forward.Cross(forwardVector);
    rotAxis = XMVector3Normalize(rotAxis);

    return CreateFromAxisAngle(rotAxis, angle);;
}