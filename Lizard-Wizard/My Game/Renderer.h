#ifndef RENDERER_H
#define RENDERER_H

#include "Defines.h"
#include "Model.h"
#include "StagedBuffer.h"
#include "GameEffect.h"
#include <Renderer3D.h>
#include <Model.h>

//NOTE(sean): A lot of this implementation is reverse-engineered based on what LSpriteRenderer does
//The DirectXTK12 docs are super helpful for all of this as well :)
class CRenderer: public LRenderer3D {
private:
    usize m_frameNumber = 0;

    StagedBuffer m_debugScratch;
    std::unique_ptr<BasicEffect> m_pDebugLineEffect;
    std::unique_ptr<BasicEffect> m_pDebugTriangleEffect;
    std::vector<DebugModel> m_debugModels;
    std::vector<ModelInstance> m_debugModelInstances;

    std::unique_ptr<GameEffect> m_pGameEffect;
    std::vector<GameModel> m_models;
    std::vector<ModelInstance> m_modelInstances;

public:
    // I dont like putting this behind walls because it doesnt stop people from fucking with it
    LBaseCamera* m_pCamera = nullptr;

    CRenderer();
    virtual ~CRenderer();

    void Initialize();

    void BeginFrame();
    void EndFrame();

    const usize GetNumFrames() const;

    void BeginDebugLineBatch();
    void BeginDebugTriangleBatch();
    void EndDebugBatch();

    //TODO(sean): look into converting these into XMMVECTOR for simd performance?
    //NOTE(sean): Put these in between BeginDebugLineBatch() and EndDebugBatch()
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

    //NOTE(sean): these are self-contained, no shimmying into a batch
    u32 LoadDebugModel(const char* name, XMVECTORF32 color);
    u32 AddDebugModel(DebugModel* model);
    void LoadAllModels();

    void DrawDebugLineModel(DebugModel* model);
    void DrawDebugTriangleModel(DebugModel* model);
    void DrawDebugModelInstance(ModelInstance* instance);

    // not sure if this is *entirely* needed
    void ResetDebugWorldMatrix();

    HWND GetHwnd();
    u32 GetResolutionWidth();
    u32 GetResolutionHeight();

    void GameEffectRenderTestCube();
    void RenderInstance(ModelInstance* instance);
    void LoadModel(const char* name, ModelType model_type);

    //TODO(sean): Implement this
    //NOTE(sean): Frustum Culling -- Don't render things behind the cameras near clip plane
    //const bool BoxInFrustum(const BoundingBox&) const;
};

#endif