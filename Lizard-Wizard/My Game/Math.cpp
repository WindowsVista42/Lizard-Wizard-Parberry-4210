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
