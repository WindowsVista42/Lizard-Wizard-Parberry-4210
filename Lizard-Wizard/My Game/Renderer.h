#ifndef RENDERER_H
#define RENDERER_H

#include "Defines.h"
#include "Model.h"
#include "StagedBuffer.h"
#include "DeferredEffect.h"
#include "LightingEffect.h"
#include "TonemapEffect.h"
#include "RenderTexture.h"
#include <Renderer3D.h>

// All the benefits of enum class, without having to cast to the type :)
namespace DeferredPass { enum e : u32 {
    Color, Normal, Position,
    Count
};}

namespace TonemapPass { enum e : u32 {
    Color,
    Count
};}

//NOTE(sean): A lot of this implementation is reverse-engineered based on what LSpriteRenderer does
//The DirectXTK12 docs are super helpful for all of this as well :)
class Renderer: public LRenderer3D {
private:
    usize m_frameNumber = 0;

    StagedBuffer m_debugScratch;
    std::unique_ptr<BasicEffect> m_pDebugLineEffect;
    std::unique_ptr<BasicEffect> m_pDebugTriangleEffect;
    std::vector<DebugModel> m_debugModels;
    std::vector<ModelInstance> m_debugModelInstances;

    std::unique_ptr<DeferredEffect> m_pDeferredEffect;
    std::unique_ptr<LightingEffect> m_pLightingEffect;
    std::unique_ptr<TonemapEffect> m_pTonemapEffect;

    std::vector<GameModel> m_models;
    std::vector<ModelInstance> m_modelInstances;

    std::unique_ptr<DescriptorHeap> m_pDeferredResourceDescs;
    std::unique_ptr<DescriptorHeap> m_pDeferredRenderDescs;
    std::vector<RenderTexture> m_deferredPassTextures;

    std::unique_ptr<DescriptorHeap> m_pTonemapResourceDescs;
    std::unique_ptr<DescriptorHeap> m_pTonemapRenderDescs;
    std::vector<RenderTexture> m_tonemapPassTextures;

public:
    bool m_screenShot = false; // TODO(sean): implement

    // I dont like putting this behind walls because it doesnt stop people from fucking with it
    LBaseCamera* m_pCamera = nullptr;

    Renderer();
    virtual ~Renderer();

    void Initialize();

    void BeginFrame();

    void BeginDrawing();
    void EndDrawing();

    void BeginDebugDrawing();
    void EndDebugDrawing();

    void EndFrame();

    const usize GetNumFrames() const;

    void BeginDebugLineBatch();
    void BeginDebugTriangleBatch();
    void EndDebugBatch();

    //TODO(sean): look into converting these into XMMVECTOR for simd performance?
    //NOTE(sean): Put these in between BeginDebugLineBatch() and EndDebugBatch()
    void DrawDebugLine(const Vec3 A, const Vec3 B, const Vec4 color);
    void DrawDebugTriangle(const Vec3 A, const Vec3 B, const Vec3 C, const Vec4 color);
    void DrawDebugQuad(const Vec3 A, const Vec3 B, const Vec3 C, const Vec3 D, const Vec4 color);
    void DrawDebugRay(const Vec3 origin, const Vec3 direction, const f32 length, const Vec4 color);
    void DrawDebugRing(const Vec3 origin, const Vec3 orientation, const f32 radius, const u32 segments, const Vec4 color);
    void DrawDebugRing2(Vec3 origin, Vec3 major, Vec3 minor, const u32 segments, const Vec4 color);
    void DrawDebugAABB(const BoundingBox box, const Vec4 color);
    void DrawDebugOBB(const BoundingOrientedBox obb, const Vec4 color);
    void DrawDebugSphere(const BoundingSphere sphere, const u32 segments, const Vec4 color);
    //void DrawDebugCylinder(const BoundingSphere sphere, const XMVECTORF32 color);
    //void DrawDebugGrid(const Vec3 center, const Vec3 orientation, const f32 length, const f32 width, const u32 length_segments, const u32 width_segments, const XMVECTORF32 color);
    void DrawDebugCapsule(const Vec3 origin, const f32 radius, const f32 height, const u32 segments, const Vec4 color);
    void DrawDebugGrid(const Vec3 x_axis, const Vec3 y_axis, const Vec3 origin, const u32 length_segments, const u32 width_segments, const Vec4 color);

    void DrawDebugCubeInternal(const Mat4x4& world, const Vec4 color);

    //NOTE(sean): these are self-contained, no shimmying into a batch
    u32 LoadDebugModel(const char* name, Vec4 color);
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

    void DrawModelInstance(ModelInstance* instance);
    void LoadModel(const char* name, ModelType model_type);

    //TODO(sean): Implement this
    //NOTE(sean): Frustum Culling -- Don't render things behind the cameras near clip plane
    //const bool BoxInFrustum(const BoundingBox&) const;
};

#endif
