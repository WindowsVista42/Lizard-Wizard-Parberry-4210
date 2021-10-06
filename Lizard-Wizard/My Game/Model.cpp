#include "Model.h"
#include "Math.h"
#include <string>
#include <Model.h>

void LoadTrianglesAsLines(const VertexPC* triangles, const u32 count, std::vector<VertexPC>* line_list) {
    // we want to grab every 2 and slap them into a line if that makes sense
    assert(count % 3 == 0);

    for(usize index = 0; index < count; index += 3) {
        line_list->push_back(triangles[index + 0]);
        line_list->push_back(triangles[index + 1]);

        line_list->push_back(triangles[index + 1]);
        line_list->push_back(triangles[index + 2]);

        line_list->push_back(triangles[index + 2]);
        line_list->push_back(triangles[index + 0]);
    }
}

DebugModel::DebugModel(const VertexPC* triangles, const u32 count) {
    m_modelType = DebugModelType::LINE_LIST;
    LoadTrianglesAsLines(triangles, count, &m_vertexList);
}

DebugModel::DebugModel(const VertexPC* triangles, const u32 count, DebugModelType type) {
    m_modelType = type;

    switch (m_modelType) {
    case(DebugModelType::LINE_LIST): {
        LoadTrianglesAsLines(triangles, count, &m_vertexList);
    } break;
    case(DebugModelType::TRIANGLE_LIST): {
        assert(count % 3 == 0);
        for every(index, count) {
            m_vertexList.push_back(triangles[index]);
        }
    } break;
    }
}

ModelInstance::ModelInstance(u32 instance) {
    m_modelIndex = instance;

    const XMMATRIX world = XMMatrixTransformation(g_XMZero, Quaternion::Identity, g_XMOne, g_XMZero, Quaternion::Identity, g_XMZero);
    m_worldMatrix = world;
}

ModelInstance::ModelInstance() {
    m_modelIndex = UINT_MAX; // i really hope you segfault if you default construct

    const XMMATRIX world = XMMatrixTransformation(g_XMZero, Quaternion::Identity, g_XMOne, g_XMZero, Quaternion::Identity, g_XMZero);
    m_worldMatrix = world;
}
