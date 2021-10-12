#ifndef MODEL_H
#define MODEL_H

#include "Defines.h"

//TODO(sean): Create real models that are VertexPNT stuff

//NOTE(sean):
enum class DebugModelType { LINE_LIST, TRIANGLE_LIST };

//NOTE(sean);
/// Basic 3d line-model
struct DebugModel {
    DebugModelType m_modelType;
    std::vector<VertexPC> m_vertexList;

    // assumes DebugModelType::LINE_LIST
    DebugModel(const VertexPC* triangles, const u32 count);
    DebugModel(const VertexPC* triangles, const u32 count, DebugModelType type);
};

//NOTE(sean):
/// Instance of a model, we store the index of the model we want to render as, and we store the model matrix
struct ModelInstance {
    u32 m_modelIndex;
    XMMATRIX m_worldMatrix;

    ModelInstance();
    ModelInstance(u32 instance);
};

enum class ModelType: u32 {
    Cube,
    Count // keep this last
};

struct GameModel {
    u32 index_count;
    GraphicsResource vertex_buffer;
    GraphicsResource index_buffer;
    std::shared_ptr<D3D12_VERTEX_BUFFER_VIEW> vertex_view;
    std::shared_ptr<D3D12_INDEX_BUFFER_VIEW> index_view;
};

#endif
