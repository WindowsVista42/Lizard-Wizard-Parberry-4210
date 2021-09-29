#include "Model.h"
#include "Math.h"

SDebugModel::SDebugModel(const VertexPC* triangles, const u32 count) {
    // we want to grab every 2 and slap them into a line if that makes sense
    assert(count % 3 == 0);

    for(usize index = 0; index < count; index += 3) {
        m_lineList.push_back(triangles[index + 0]);
        m_lineList.push_back(triangles[index + 1]);

        m_lineList.push_back(triangles[index + 1]);
        m_lineList.push_back(triangles[index + 2]);

        m_lineList.push_back(triangles[index + 2]);
        m_lineList.push_back(triangles[index + 0]);
    }
}

SModelInstance::SModelInstance(u32 instance) {
    m_modelIndex = instance;

    const XMMATRIX world = XMMatrixTransformation(g_XMZero, Quaternion::Identity, g_XMOne, g_XMZero, Quaternion::Identity, g_XMZero);
    m_worldMatrix = world;
}

SModelInstance::SModelInstance() {
    m_modelIndex = UINT_MAX; // i really hope you segfault if you default construct

    const XMMATRIX world = XMMatrixTransformation(g_XMZero, Quaternion::Identity, g_XMOne, g_XMZero, Quaternion::Identity, g_XMZero);
    m_worldMatrix = world;
}

