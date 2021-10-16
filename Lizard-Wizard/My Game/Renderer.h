#ifndef RENDERER_H
#define RENDERER_H

#include "Defines.h"
#include "Model.h"
#include "StagedBuffer.h"
#include "GameEffect.h"
#include "PostProcessEffect.h"
#include <Renderer3D.h>
#include <Model.h>

struct RenderTexture {
    RenderTexture():
        m_state(D3D12_RESOURCE_STATE_COMMON),
        m_srvDescriptor{},
        m_rtvDescriptor{},
        m_format(DXGI_FORMAT_UNKNOWN),
        m_width(0),
        m_height(0),
        m_clearColor(Colors::Black)
    {}

    RenderTexture(DXGI_FORMAT format, Vec4 clear_color):
        m_state(D3D12_RESOURCE_STATE_COMMON),
        m_srvDescriptor{},
        m_rtvDescriptor{},
        m_format(format),
        m_width(0),
        m_height(0),
        m_clearColor(clear_color)
    {}

    void UpdateDescriptors(
        D3D12_CPU_DESCRIPTOR_HANDLE srvDescriptor,
        D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptor
    ) {
        if (!srvDescriptor.ptr || !rtvDescriptor.ptr) {
            ABORT_EQ_FORMAT(0, 0, "Invalid descriptors");
        }

        this->m_srvDescriptor = srvDescriptor;
        this->m_rtvDescriptor = rtvDescriptor;
    }

    void UpdateResources(
        ID3D12Device* device,
        usize width,
        usize height
    ) {
        if (this->m_width == width && this->m_height == height) {
            return;
        }

        auto heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        D3D12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Tex2D(
            m_format,
            (u64)width,
            (u32)height,
            1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
        );

        D3D12_CLEAR_VALUE clear_value = { m_format, {} };
        memcpy(clear_value.Color, &m_clearColor, sizeof(clear_value.Color));

        m_state = D3D12_RESOURCE_STATE_RENDER_TARGET;

        // Create the render target
        ThrowIfFailed(
            device->CreateCommittedResource(
                &heap_properties,
                D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
                &resource_desc,
                m_state, 
                &clear_value,
                IID_GRAPHICS_PPV_ARGS(m_resource.ReleaseAndGetAddressOf())
            )
        );

        // Create RTV
        device->CreateRenderTargetView(m_resource.Get(), 0, m_rtvDescriptor);

        // Create SRV
        device->CreateShaderResourceView(m_resource.Get(), 0, m_srvDescriptor);

        this->m_width = width;
        this->m_height = height;
    }

    void TransitionTo(
        ID3D12GraphicsCommandList* command_list,
        D3D12_RESOURCE_STATES after_state
    ) {
        TransitionResource(command_list, m_resource.Get(), m_state, after_state);
        m_state = after_state;
    }

    Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
    D3D12_RESOURCE_STATES m_state;
    D3D12_CPU_DESCRIPTOR_HANDLE m_srvDescriptor;
    D3D12_CPU_DESCRIPTOR_HANDLE m_rtvDescriptor;
    DXGI_FORMAT m_format;
    Vec4 m_clearColor;
    usize m_width;
    usize m_height;
};

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
    std::unique_ptr<PostProcessEffect> m_pPostProcessEffect;
    std::vector<GameModel> m_models;
    std::vector<ModelInstance> m_modelInstances;

    std::unique_ptr<DescriptorHeap> m_pDeferredResourceDescs;
    std::unique_ptr<DescriptorHeap> m_pDeferredRenderDescs;
    std::vector<RenderTexture> m_deferredPassTextures;

    enum class OutputAttachment: u32 {
        Diffuse, Normal, Position,
        Count, AllCount
    };

public:
    bool m_screenShot = false; // TODO(sean): implement

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