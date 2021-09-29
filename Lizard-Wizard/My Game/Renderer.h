#ifndef RENDERER_H
#define RENDERER_H

#include "Defines.h"
#include "Model.h"
#include "StagedBuffer.h"
#include <Renderer3D.h>

//NOTE(sean): A lot of this implementation is reverse-engineered based on what LSpriteRenderer does
class CRenderer: public LRenderer3D {
private:
    usize m_frameNumber = 0;

    /*
        TODO(sean): Add member variables
    */

    /// This is a basic line rendering effect
    std::unique_ptr<BasicEffect> m_pDebugEffect;
    void CreateAllEffects();

    GraphicsResource m_cubeVertexBuffer;
    GraphicsResource m_cubeIndexBuffer;
    //TODO(sean): do these really need to be pointers???
    std::shared_ptr<D3D12_VERTEX_BUFFER_VIEW> m_pVertexBufferView;
    std::shared_ptr<D3D12_INDEX_BUFFER_VIEW> m_pIndexBufferView;
    void CreateCubeBuffers();

    StagedBuffer m_debugScratch; // this expects to be cleared after every function call that uses it

    //TODO(sean): do this same thing, but on the gpu
    std::vector<SDebugModel> m_debugModels;
    std::vector<SModelInstance> m_debugModelInstances;

public:
    // I dont like putting this behind walls because it doesnt stop people from fucking with it
    LBaseCamera* m_pCamera = nullptr;

    CRenderer();
    virtual ~CRenderer();

    void Initialize();

    void BeginFrame();
    void EndFrame();

    const usize GetNumFrames() const;

    void BeginDebugDrawing();
    void EndDebugDrawing();

    //TODO(sean): look into converting these into XMMVECTOR for simd performance?
    void DrawDebugLine(const Vector3 A, const Vector3 B, const XMVECTORF32 color);
    void DrawDebugTriangle(const Vector3 A, const Vector3 B, const Vector3 C, const XMVECTORF32 color);
    void DrawDebugQuad(const Vector3 A, const Vector3 B, const Vector3 C, const Vector3 D, const XMVECTORF32 color);
    void DrawDebugRay(const Vector3 origin, const Vector3 direction, const f32 length, const XMVECTORF32 color);
    void DrawDebugRing(const Vector3 origin, const Vector3 orientation, const f32 radius, const u32 segments, const XMVECTORF32 color);
    void DrawDebugRing2(FXMVECTOR origin, FXMVECTOR major, FXMVECTOR minor, const u32 segments, const XMVECTORF32 color);
    void DrawDebugAABB(const BoundingBox box, const XMVECTORF32 color);
    void DrawDebugOBB(const BoundingOrientedBox obb, const XMVECTORF32 color);
    void DrawDebugSphere(const BoundingSphere sphere, const u32 segments, const XMVECTORF32 color);
    //void DrawDebugCylinder(const BoundingSphere sphere, const XMVECTORF32 color);
    //void DrawDebugCapsule(const BoundingSphere sphere, const XMVECTORF32 color);
    //void DrawDebugGrid(const Vector3 center, const Vector3 orientation, const f32 length, const f32 width, const u32 length_segments, const u32 width_segments, const XMVECTORF32 color);
    void DrawDebugCapsule(const Vector3 origin, const f32 radius, const f32 height, const u32 segments, const XMVECTORF32 color);
    void DrawDebugGrid(const Vector3 x_axis, const Vector3 y_axis, const Vector3 origin, const u32 length_segments, const u32 width_segments, const XMVECTORF32 color);

    void DrawDebugCubeInternal(const CXMMATRIX world, const XMVECTORF32 color);

    u32 AddDebugModel(SDebugModel* model);
    void DrawDebugModel(SDebugModel* model);
    void DrawDebugModelInstance(SModelInstance* instance);

    HWND GetHwnd();
    u32 GetResolutionWidth();
    u32 GetResolutionHeight();

    //TODO(sean): Implement this
    //NOTE(sean): Frustum Culling -- Don't render things behind the cameras near clip plane
    //const bool BoxInFrustum(const BoundingBox&) const;
};

#endif