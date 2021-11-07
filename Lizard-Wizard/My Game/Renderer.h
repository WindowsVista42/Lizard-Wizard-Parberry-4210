#ifndef RENDERER_H
#define RENDERER_H

#include "Defines.h"
#include "Model.h"
#include "StagedBuffer.h"

#include "Effects/DeferredEffect.h"
#include "Effects/LightingEffect.h"
#include "Effects/TonemapEffect.h"
#include "Effects/BloomEffect.h"

#include "RenderTexture.h"
#include "Ecs.h"
#include <Renderer3D.h>
#include <array>

#include <PostProcess.h>

#define MAX_LIGHTS 254
#define BLOOM_PASS_COUNT 4

namespace Descriptors { enum e : u32 {
    DeferredColor, // <-- Input
    DeferredNormal, // <-- Input
    DeferredPosition, // <-- Input
    LightingColor, // <-- DeferredColor + DeferredNormal + DeferredPosition
    BloomCombine0, // <-- Blur1 + Combine1
    BloomExtract, // <-- Lighting
    BloomBlur0, // <-- Extract
    BloomBlur1, // <-- Blur0
    BloomCombine1, // <-- Blur2 + Combine2
    BloomBlur2, // <-- Blur1
    BloomCombine2, // <-- Blur3 + Blur4
    BloomBlur3, // <-- Blur2
    BloomCombine3, // <-- Blur3 + Blur4
    BloomBlur4, // <-- Blur3
    BloomBlur5, // <-- Blur3
    BloomOutput, // <-- Lighting + Combine0 | Output --> Tonemap --> Backbuffer
    Count
};}

struct Particle {
    Vec3 pos;
    Vec3 vel;
    Vec3 acc;
};

struct ParticleInstanceDesc {
    Vec3 origin;
    f32 initial_speed;
    Vec3 light_color;
    Vec3 model_scale;
    u32 model;
    u32 texture;
    Vec3 solid_color;
    u32 count;
    f32 randomness;
};

struct ParticleInstance {
    Group particles;
    Vec3 model_scale;
    Vec3 solid_color;
    Entity light;
    ModelInstance particle_instance;
};

//NOTE(sean): A lot of this implementation is reverse-engineered based on what LSpriteRenderer does
//The DirectXTK12 docs are super helpful for all of this as well :)
class Renderer: public LRenderer3D {
private:
    usize m_frameNumber = 0;

    // Normal Rendering 
    std::unique_ptr<DeferredEffect> m_deferred;
    std::unique_ptr<LightingEffect> m_lighting;
    std::unique_ptr<BloomExtractEffect> m_bloomExtract;
    std::unique_ptr<BloomBlurEffect> m_bloomBlur;
    std::unique_ptr<BloomCombineEffect> m_bloomCombine;
    std::unique_ptr<TonemapEffect> m_pTonemapEffect;

    std::unique_ptr<DescriptorHeap> m_pResourcesHeap;
    std::unique_ptr<DescriptorHeap> m_pRendersHeap;
    std::array<RenderTexture, Descriptors::Count> m_renderTextures;

    // Debug Rendering
    StagedBuffer m_debugScratch;
    std::unique_ptr<BasicEffect> m_pDebugLineEffect;
    std::unique_ptr<BasicEffect> m_pDebugTriangleEffect;

    // Text Rendering

    void BetterScreenShot();

    std::vector<DebugModel> m_debugModels;
    std::array<GameModel, ModelIndex::Count> m_models;

public:
    bool m_screenShot = false; // TODO(sean): implement

    Vec3 tint_color = Vec3(0.0f, 0.0f, 0.0f);
    f32 blur_amount = 0.0f;
    f32 saturation_amount = 1.0f;

    // I dont like putting this behind walls because it doesnt stop people from fucking with it
    LBaseCamera* m_pCamera = nullptr;

    Table<Light> lights;
    Table<Particle> particles;

    void UpdateParticles();

    Renderer();
    virtual ~Renderer();

    void Initialize();

    void BeginFrame();

    void BeginDrawing();
    void EndDrawing();

    void BeginDebugDrawing();
    void EndDebugDrawing();

    void BeginUIDrawing();
    void EndUIDrawing();

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
    void DrawDebugCapsule(const Vec3 origin, const f32 radius, const f32 height, const u32 segments, const Vec4 color);
    void DrawDebugGrid(const Vec3 x_axis, const Vec3 y_axis, const Vec3 origin, const u32 length_segments, const u32 width_segments, const Vec4 color);

    void DrawDebugCubeInternal(const Mat4x4& world, const Vec4 color);

    //NOTE(sean): these are self-contained, no shimmying into a batch
    u32 LoadDebugModel(const char* name, Vec4 color);
    u32 AddDebugModel(DebugModel* model);

    void DrawDebugLineModel(DebugModel* model);
    void DrawDebugTriangleModel(DebugModel* model);
    void DrawDebugModelInstance(ModelInstance* instance);

    // not sure if this is *entirely* needed
    void ResetDebugWorldMatrix();

    HWND GetHwnd();
    u32 GetResolutionWidth();
    u32 GetResolutionHeight();

    ParticleInstance CreateParticleInstance(ParticleInstanceDesc* desc);
    void DrawModelInstance(ModelInstance* instance);
    void DrawParticleInstance(ParticleInstance* instance);
    void LoadModel(const char* name, u32 model_index);
    void LoadTextureI(const char* name, u32 texture_index);

    //TODO(sean): Implement this
    //NOTE(sean): Frustum Culling -- Don't render things behind the cameras near clip plane
    //const bool BoxInFrustum(const BoundingBox&) const;
};

#endif
