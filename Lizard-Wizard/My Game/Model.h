#ifndef MODEL_H
#define MODEL_H

#include "Defines.h"

//TODO(sean): Create real models that are VertexPNT stuff

//NOTE(sean);
/// Basic 3d line-model
struct SDebugModel {
    std::vector<VertexPC> m_lineList;

    SDebugModel(const VertexPC* data, const u32 count);
    //void FromLineList(const VertexPC* line_list);
};

//NOTE(sean):
/// Instance of a model, we store the index of the model we want to render as, and we store the model matrix
struct SModelInstance {
    u32 m_modelIndex;
    XMMATRIX m_worldMatrix; // i dont know if this should be called model or world matrix

    SModelInstance(u32 instance);
};

#endif
