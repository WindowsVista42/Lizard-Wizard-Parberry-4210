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
#include <array>

// All the benefits of enum class, without having to cast to the type :)
namespace DeferredOutput { enum e : u32 {
    Color, Normal, Position,
    Count
};}

namespace LightingOutput { enum e : u32 {
    Color,
    Count
};}

template <typename E, const usize C>
struct RenderPass {
    std::unique_ptr<E> effect;
    std::unique_ptr<DescriptorHeap> resources;
    std::unique_ptr<DescriptorHeap> renders;
    std::array<RenderTexture, C> textures;

    void InitDescs(ID3D12Device* command_list, usize width, usize height) {
        for every(index, C) {
        	textures[index].Init(
        	    command_list, 
        	    resources->GetCpuHandle(index),
        	    renders->GetCpuHandle(index),
        	    width,
        	    height 
        	);
        }
    }

    void SetAsInput(ID3D12GraphicsCommandList* command_list) {
    	ID3D12DescriptorHeap* heaps[] = { resources->Heap() };
    	command_list->SetDescriptorHeaps(_countof(heaps), heaps);

        for every(index, C) {
			textures[index].TransitionTo(command_list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        }
    }

    void SetAsOutput(ID3D12GraphicsCommandList* command_list) {
        std::array<CD3DX12_CPU_DESCRIPTOR_HANDLE, C> descriptors;
        for every(index, C) {
            descriptors[index] = renders->GetCpuHandle(index);
        }

        command_list->OMSetRenderTargets(C, descriptors.data(), FALSE, 0);

        for every(index, C) {
			textures[index].TransitionTo(command_list, D3D12_RESOURCE_STATE_RENDER_TARGET);
        }
    }

    void SetAsOutput(ID3D12GraphicsCommandList* command_list, const CD3DX12_CPU_DESCRIPTOR_HANDLE dsvDescriptor) {
        std::array<CD3DX12_CPU_DESCRIPTOR_HANDLE, C> descriptors;
        for every(index, C) {
            descriptors[index] = renders->GetCpuHandle(index);
        }

        command_list->OMSetRenderTargets(C, descriptors.data(), FALSE, &dsvDescriptor);

        for every(index, C) {
			textures[index].TransitionTo(command_list, D3D12_RESOURCE_STATE_RENDER_TARGET);
        }
    }

    void ClearRenderTargetViews(ID3D12GraphicsCommandList* command_list) {
        for every(index, C) {
            D3D12_CPU_DESCRIPTOR_HANDLE handle = renders->GetCpuHandle(index);
            Vec4 clear_color = textures[index].m_clearColor;
            command_list->ClearRenderTargetView(handle, clear_color, 0, 0);
        }
    }

    void CreateHeaps(ID3D12Device* device) {
   		resources = std::make_unique<DescriptorHeap>(
   		    device, C
   		);
		
   		renders = std::make_unique<DescriptorHeap>(
   		    device,
   		    D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
   		    D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            C
   		);
    }
};

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

    std::unique_ptr<TonemapEffect> m_pTonemapEffect;
    RenderPass<DeferredEffect, DeferredOutput::Count> m_deferred;
    RenderPass<LightingEffect, LightingOutput::Count> m_lighting;

    std::vector<GameModel> m_models;
    std::vector<ModelInstance> m_modelInstances;

    void BetterScreenShot();

public:
    bool m_screenShot = false; // TODO(sean): implement

    Vec3 tint_color;
    f32 blur_amount;
    f32 saturation_amount;

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

    void DrawModelInstance(ModelInstance* instance);
    void LoadModel(const char* name, u32 model_index);

    //TODO(sean): Implement this
    //NOTE(sean): Frustum Culling -- Don't render things behind the cameras near clip plane
    //const bool BoxInFrustum(const BoundingBox&) const;
};

#endif
