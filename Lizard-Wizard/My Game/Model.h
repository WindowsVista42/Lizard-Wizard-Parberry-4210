#ifndef MODEL_H
#define MODEL_H

#include "Defines.h"

//TODO(sean): Create real models that are VertexPNT stuff

//NOTE(sean):
enum DebugModelType { LINE_LIST, TRIANGLE_LIST };

//NOTE(sean);
/// Basic 3d line-model
struct SDebugModel {
    DebugModelType m_modelType;
    std::vector<VertexPC> m_vertexList;

    // assumes DebugModelType::LINE_LIST
    SDebugModel(const VertexPC* triangles, const u32 count);
    SDebugModel(const VertexPC* triangles, const u32 count, DebugModelType type);
};

//NOTE(sean):
/// Instance of a model, we store the index of the model we want to render as, and we store the model matrix
struct SModelInstance {
    u32 m_modelIndex;
    XMMATRIX m_worldMatrix;

    SModelInstance();
    SModelInstance(u32 instance);
};

#endif
