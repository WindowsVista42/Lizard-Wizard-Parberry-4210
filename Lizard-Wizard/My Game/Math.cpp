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

btQuaternion CalculateVelocity(Vec3 origin, Vec3 target, f32 time) {
    // Distance Calculation
    Vec3 distance = target - origin;
    Vec3 distanceXZ = distance;

    // Velocity Calculation
    f32 distanceY = distance.y;
    f32 magnitudeXZ = distanceXZ.x * distanceXZ.x + distanceXZ.y * distanceXZ.y + distanceXZ.z * distanceXZ.z;
    f32 velocityXZ = magnitudeXZ / time;
    f32 velocityY = distanceY / time + 0.5f * abs(-5000.0f) * time;

    // Normalize Vector
    Vec3 resultVector = XMVector3Normalize(distanceXZ);
    resultVector *= velocityXZ;
    resultVector.y = velocityY;

    // Quaternion
    btQuaternion newRotation;
    newRotation.setEulerZYX(resultVector.x, resultVector.y, resultVector.z);

    return newRotation;
}

Vec3 RandomPointIn2DPlane(f32 bounding) {

    // Randomize
    f32 xnoise = bounding * (GameRandom::Randf32() - 0.5f) * 2.0f;
    f32 znoise = bounding * (GameRandom::Randf32() - 0.5f) * 2.0f;



    return Vec3(xnoise, 0.0f, znoise);
}

Vec3 BiasedPointIn2DPlane(f32 bounding, Vec3 origin, Vec3 bias) {
    Vec3 direction = bias - origin;

    direction.Normalize();

    direction = JitterVec3(direction, -bounding, bounding);

    return Vec3(direction.x, 0.0f, direction.z);
}

Vec3 RotatePointAroundOrigin(Vec3 origin, Vec3 point, Quat quat) {
   Mat4x4 rotation = XMMatrixRotationQuaternion(quat);
   return origin + Vec3(XMVector3Transform(point, rotation));
}

Quat GetRotationFromTwoVectors(Vec3 vec1, Vec3 vec2) {
    vec1 = XMVector3Normalize(vec1);
    vec2 = XMVector3Normalize(vec2);

    if (vec1 == -vec2) {
        Vec3 returnVec = XMVector3Normalize(XMVector3Orthogonal(vec1));
        return Quat(0.0f, returnVec.x, returnVec.y, returnVec.z);
    }

    Vec3 half = XMVector3Normalize(vec1 + vec2);
    Vec3 cross = XMVector3Cross(vec1, half);
    f32 dot = vec1.Dot(half);

    return Quat(dot, cross.x, cross.y, cross.z);
}

f32 DistanceBetweenVectors(Vec3 vec1, Vec3 vec2) {
    return sqrt(pow(vec2.x - vec1.x, 2) + pow(vec2.y - vec1.y, 2) + pow(vec2.z - vec1.z, 2));
}

Quat QuatLookAt(Vec3 origin, Vec3 point) {
    Vec3 toVector = XMVector3Normalize(point - origin);

    //compute rotation axis
    Vec3 rotAxis = XMVector3Normalize(XMVector3Cross(Vec3(1.0f, 0.0f, 0.0f), toVector));
    Vec3 normalizedAxis = XMVector3Normalize(rotAxis);
    if (normalizedAxis * normalizedAxis == Vec3::Zero)
        rotAxis = Vec3(0.0f, 1.0f, 0.0f);

    //find the angle around rotation axis
    f32 dot = Vec3(1.0f, 0.0f, 0.0f).Dot(toVector);
    f32 ang = acosf(dot);

    //convert axis angle to quaternion
    return AngleAxisf(rotAxis, ang);
}

Quat AngleAxisf(Vec3 axis, f32 angle) {
    f32 s = sinf(angle / 2.0f);
    Vec3 u = XMVector3Normalize(axis);
    return Quat(cosf(angle / 2.0f), u.x * s, u.y * s, u.z * s);
}