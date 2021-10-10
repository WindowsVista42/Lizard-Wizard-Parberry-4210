#include "Math.h"

XMMATRIX MoveRotateScaleMatrix(Vector3 position, Quaternion rotation, Vector3 scale) {
    XMMATRIX world = XMMatrixRotationQuaternion(XMLoadFloat4(&rotation));
    XMMATRIX scl = XMMatrixScaling(scale.x, scale.y, scale.z);
    world = XMMatrixMultiply(scl, world);
    XMVECTOR pos = XMLoadFloat3(&position);
    world.r[3] = XMVectorSelect(world.r[3], pos, g_XMSelect1110);

    return world;
}

XMMATRIX MoveScaleMatrix(Vector3 position, Vector3 scale) {
    XMMATRIX world = XMMatrixScaling(scale.x, scale.y, scale.z); // scale the world matrix
    XMVECTOR pos = XMLoadFloat3(&position);
    world.r[3] = XMVectorSelect(world.r[3], pos, g_XMSelect1110); // position the world matrix

    return world;
}

Vector3 JitterVector3(Vector3 input, f32 negativeAccuracy, f32 range) {
    return Vector3(
        input.x + ((negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.),
        input.y + ((negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.),
        input.z + ((negativeAccuracy + double((range + 1.0) * rand() / (RAND_MAX + 1.0))) / 100.)
    );
}
