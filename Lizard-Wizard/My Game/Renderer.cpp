#include "Renderer.h"
#include "Math.h"
#include "Helpers.h"
#include <iostream>
#include "Keyboard.h"
#include <ScreenGrab.h>
#include <BufferHelpers.h>
#include <ComponentIncludes.h>

Renderer::Renderer() :
    LRenderer3D(),
    m_pCamera(new LBaseCamera),
    m_debugScratch(16 * 1024), // 16k
    m_models()
{
    //NOTE(sean): Windows window stuff
    m_f32BgColor = Colors::Black; // NOTE(sean): set the clear color
    ShowCursor(0);

    //NOTE(sean): camera stuff
    const f32 width = (f32)m_nWinWidth;
    const f32 height = (f32)m_nWinHeight;
    const f32 aspect = width / height;

    //TODO(sean): Load this from settings
    const f32 fov = 90.0;
    const f32 fov_radians = (fov / 180.0) * XM_PI;

    const f32 near_clip = 0.1f;
    const f32 far_clip = 1000000.0f;
    
    m_pCamera->SetPerspective(aspect, fov_radians, near_clip, far_clip);
    m_pCamera->MoveTo(Vec3(0.0f));
}

Renderer::~Renderer() {
    delete m_pCamera;
    m_pDeviceResources->WaitForGpu();
}

void Renderer::Initialize() {
    LRenderer3D::Initialize(true);

    {
        m_pResourcesHeap = std::make_unique<DescriptorHeap>(
            m_pD3DDevice,
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
            Descriptors::Count
        );

        m_pRendersHeap = std::make_unique<DescriptorHeap>(
            m_pD3DDevice,
            D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            Descriptors::Count
        );

        // Deferred Pass Output RT
        m_renderTextures[Descriptors::DeferredColor] = RenderTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black);
        m_renderTextures[Descriptors::DeferredNormal] = RenderTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black);
        m_renderTextures[Descriptors::DeferredPosition] = RenderTexture(DXGI_FORMAT_R32G32B32A32_FLOAT, Colors::Black);

        // Lighting Pass Output RT
        m_renderTextures[Descriptors::LightingColor] = RenderTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black); 

        // Bloom Pass Output RT
        m_renderTextures[Descriptors::BloomCombine0] = RenderTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black);
        m_renderTextures[Descriptors::BloomExtract] = RenderTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black);
        m_renderTextures[Descriptors::BloomBlur0] = RenderTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black);
        m_renderTextures[Descriptors::BloomBlur1] = RenderTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black);
        m_renderTextures[Descriptors::BloomCombine1] = RenderTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black);
        m_renderTextures[Descriptors::BloomBlur2] = RenderTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black);
        m_renderTextures[Descriptors::BloomCombine2] = RenderTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black);
        m_renderTextures[Descriptors::BloomBlur3] = RenderTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black);
        m_renderTextures[Descriptors::BloomCombine3] = RenderTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black);
        m_renderTextures[Descriptors::BloomBlur4] = RenderTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black);
        m_renderTextures[Descriptors::BloomBlur5] = RenderTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black);
        m_renderTextures[Descriptors::BloomOutput] = RenderTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black);

        // Deferred + Lighting Pass Output Res
        for every(index, Descriptors::LightingColor + 1) {
            m_renderTextures[index].Init(
                m_pD3DDevice, 
                m_pResourcesHeap->GetCpuHandle(index), 
                m_pRendersHeap->GetCpuHandle(index), 
                m_nWinWidth, 
                m_nWinHeight
            );
        }

        #define FILL(device, res, ren, rt, index, width, height) \
        rt[index].Init(device, res->GetCpuHandle(index), ren->GetCpuHandle(index), width, height);

        // Bloom Pass Output Res
        u32 resx = m_nWinWidth;
        u32 resy = m_nWinHeight;

        FILL(m_pD3DDevice, m_pResourcesHeap.get(), m_pRendersHeap.get(), m_renderTextures.data(), Descriptors::BloomOutput, resx, resy);
        FILL(m_pD3DDevice, m_pResourcesHeap.get(), m_pRendersHeap.get(), m_renderTextures.data(), Descriptors::BloomExtract, resx, resy);
        FILL(m_pD3DDevice, m_pResourcesHeap.get(), m_pRendersHeap.get(), m_renderTextures.data(), Descriptors::BloomBlur0, resx, resy);
        FILL(m_pD3DDevice, m_pResourcesHeap.get(), m_pRendersHeap.get(), m_renderTextures.data(), Descriptors::BloomCombine0, resx, resy);

        resx /= 2; resy /= 2;
        FILL(m_pD3DDevice, m_pResourcesHeap.get(), m_pRendersHeap.get(), m_renderTextures.data(), Descriptors::BloomBlur1, resx, resy);
        FILL(m_pD3DDevice, m_pResourcesHeap.get(), m_pRendersHeap.get(), m_renderTextures.data(), Descriptors::BloomCombine1, resx, resy);

        resx /= 2; resy /= 2;
        FILL(m_pD3DDevice, m_pResourcesHeap.get(), m_pRendersHeap.get(), m_renderTextures.data(), Descriptors::BloomBlur2, resx, resy);
        FILL(m_pD3DDevice, m_pResourcesHeap.get(), m_pRendersHeap.get(), m_renderTextures.data(), Descriptors::BloomCombine2, resx, resy);

        resx /= 2; resy /= 2;
        FILL(m_pD3DDevice, m_pResourcesHeap.get(), m_pRendersHeap.get(), m_renderTextures.data(), Descriptors::BloomCombine3, resx, resy);
        FILL(m_pD3DDevice, m_pResourcesHeap.get(), m_pRendersHeap.get(), m_renderTextures.data(), Descriptors::BloomBlur3, resx, resy);

        resx /= 2; resy /= 2;
        FILL(m_pD3DDevice, m_pResourcesHeap.get(), m_pRendersHeap.get(), m_renderTextures.data(), Descriptors::BloomBlur4, resx, resy);

        resx /= 2; resy /= 2;
        FILL(m_pD3DDevice, m_pResourcesHeap.get(), m_pRendersHeap.get(), m_renderTextures.data(), Descriptors::BloomBlur5, resx, resy);
    }

    // This is very similar to vulkan :)
    {
        EffectPipelineStateDescription pipeline_state_desc(
            &VertexPC::InputLayout,
            CommonStates::Opaque,
            CommonStates::DepthDefault,
            CommonStates::CullNone,
            m_RenderTargetState,
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE
        );

        m_pDebugLineEffect = std::make_unique<BasicEffect>(m_pD3DDevice, EffectFlags::VertexColor, pipeline_state_desc);
        m_pDebugLineEffect->SetProjection(XMLoadFloat4x4(&m_projection));
    }

    {
        EffectPipelineStateDescription pipeline_state_desc(
            &VertexPC::InputLayout,
            CommonStates::Opaque,
            CommonStates::DepthDefault,
            CommonStates::CullNone,
            m_RenderTargetState,
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
        );

        m_pDebugTriangleEffect = std::make_unique<BasicEffect>(m_pD3DDevice, EffectFlags::VertexColor, pipeline_state_desc);
        m_pDebugTriangleEffect->SetProjection(XMLoadFloat4x4(&m_projection));
    }

    {
        RenderTargetState render_target_state = {};
        render_target_state.dsvFormat = m_pDeviceResources->GetDepthBufferFormat();
        render_target_state.numRenderTargets = 3;
        render_target_state.rtvFormats[0] = m_renderTextures[Descriptors::DeferredColor].m_format;
        render_target_state.rtvFormats[1] = m_renderTextures[Descriptors::DeferredNormal].m_format;
        render_target_state.rtvFormats[2] = m_renderTextures[Descriptors::DeferredPosition].m_format;
        render_target_state.sampleMask = ~0u;
        render_target_state.sampleDesc.Count = 1;

        EffectPipelineStateDescription pipeline_state_desc(
            &VertexPNT::InputLayout,
            CommonStates::Opaque,
            CommonStates::DepthDefault,
            CommonStates::CullCounterClockwise,
            render_target_state
        );

        m_deferred = std::make_unique<DeferredEffect>(m_pD3DDevice, pipeline_state_desc);
        m_deferred->SetProjection(XMLoadFloat4x4(&m_projection));
    }

    {
        RenderTargetState render_target_state = {};
        render_target_state.dsvFormat = m_pDeviceResources->GetDepthBufferFormat();
        render_target_state.numRenderTargets = 1;
        render_target_state.rtvFormats[0] = m_renderTextures[Descriptors::LightingColor].m_format;
        render_target_state.sampleMask = ~0u;
        render_target_state.sampleDesc.Count = 1;

        EffectPipelineStateDescription pipeline_state_desc(
            0,
            CommonStates::Opaque,
            CommonStates::DepthNone,
            CommonStates::CullNone,
            render_target_state
        );

        m_lighting = std::make_unique<LightingEffect>(m_pD3DDevice, pipeline_state_desc);
        m_lighting->SetTextures(m_pResourcesHeap->GetGpuHandle(Descriptors::DeferredColor));
    }

    {
        RenderTargetState render_target_state = {};
        render_target_state.dsvFormat = m_pDeviceResources->GetDepthBufferFormat();
        render_target_state.numRenderTargets = 1;
        render_target_state.rtvFormats[0] = m_renderTextures[Descriptors::BloomExtract].m_format;
        render_target_state.sampleMask = ~0u;
        render_target_state.sampleDesc.Count = 1;

        EffectPipelineStateDescription pipeline_state_desc(
            0,
            CommonStates::Opaque,
            CommonStates::DepthNone,
            CommonStates::CullNone,
            render_target_state
        );

        m_bloomExtract = std::make_unique<BloomExtractEffect>(m_pD3DDevice, pipeline_state_desc);
        m_bloomExtract->SetTextures(m_pResourcesHeap->GetGpuHandle(Descriptors::LightingColor));
    }

    {
        RenderTargetState render_target_state = {};
        render_target_state.dsvFormat = m_pDeviceResources->GetDepthBufferFormat();
        render_target_state.numRenderTargets = 1;
        render_target_state.rtvFormats[0] = m_renderTextures[Descriptors::BloomBlur0].m_format;
        render_target_state.sampleMask = ~0u;
        render_target_state.sampleDesc.Count = 1;

        EffectPipelineStateDescription pipeline_state_desc(
            0,
            CommonStates::Opaque,
            CommonStates::DepthNone,
            CommonStates::CullNone,
            render_target_state
        );

        m_bloomBlur = std::make_unique<BloomBlurEffect>(m_pD3DDevice, pipeline_state_desc);
    }

    {
        RenderTargetState render_target_state = {};
        render_target_state.dsvFormat = m_pDeviceResources->GetDepthBufferFormat();
        render_target_state.numRenderTargets = 1;
        render_target_state.rtvFormats[0] = m_renderTextures[Descriptors::BloomCombine0].m_format;
        render_target_state.sampleMask = ~0u;
        render_target_state.sampleDesc.Count = 1;

        EffectPipelineStateDescription pipeline_state_desc(
            0,
            CommonStates::Opaque,
            CommonStates::DepthNone,
            CommonStates::CullNone,
            render_target_state
        );

        m_bloomCombine = std::make_unique<BloomCombineEffect>(m_pD3DDevice, pipeline_state_desc);
    }

    {
        EffectPipelineStateDescription pipeline_state_desc(
            0,
            CommonStates::Opaque,
            CommonStates::DepthNone,
            CommonStates::CullNone,
            m_RenderTargetState
        );

        m_pTonemapEffect = std::make_unique<TonemapEffect>(m_pD3DDevice, pipeline_state_desc);
        m_pTonemapEffect->SetTextures(m_pResourcesHeap->GetGpuHandle(Descriptors::BloomOutput));
    }

    {
        EffectPipelineStateDescription pipeline_state_desc(
            &VertexPositionTexture::InputLayout,
            CommonStates::NonPremultiplied,
            CommonStates::DepthDefault,
            CommonStates::CullNone,
            m_RenderTargetState
        );
        m_spriteEffect = std::make_unique<BasicEffect>(m_pD3DDevice, EffectFlags::Texture, pipeline_state_desc);
        m_spriteEffect->SetProjection(XMLoadFloat4x4(&m_projection));
    }

    {
        // Sean: adaptation of LSpriteRenderer::CreateVertexBuffer()

        VertexPositionTexture vertex[4] = {
            { Vec3( 0.5f,  0.5f, 0.0f), Vec2(1.0f, 0.0f) },
            { Vec3( 0.5f, -0.5f, 0.0f), Vec2(1.0f, 1.0f) },
            { Vec3(-0.5f,  0.5f, 0.0f), Vec2(0.0f, 0.0f) },
            { Vec3(-0.5f, -0.5f, 0.0f), Vec2(0.0f, 1.0f) },
        };
        CreateBufferAndView(vertex, _countof(vertex), m_spriteVertexBuffer, m_spriteVertexBufferView);

        u32 index[4] = { 0, 1, 2, 3 };
        CreateBufferAndView(index, _countof(index), m_spriteIndexBuffer, m_spriteIndexBufferView);
    }
}

void Renderer::BetterScreenShot() {
    SYSTEMTIME st;
    GetSystemTime(&st);

    const std::wstring file_name =
        L"Lizard Wizard " +
        std::to_wstring(st.wYear) + L"." + std::to_wstring(st.wMonth) + L"." + std::to_wstring(st.wDay) + L" " +
        std::to_wstring(st.wHour) + L"." + std::to_wstring(st.wMinute) + L"." + std::to_wstring(st.wSecond) + L"." + std::to_wstring(st.wMilliseconds) + L".png";

    SaveWICTextureToFile(
        m_pDeviceResources->GetCommandQueue(),
        m_pDeviceResources->GetRenderTarget(),
        GUID_ContainerFormatPng,
        file_name.c_str(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_PRESENT
    );
}

void Renderer::BeginFrame() {
    //NOTE(sean): Tell DirectX that we want to create a CommandList for the GPU
    m_pDeviceResources->Prepare();
    m_pCommandList = m_pDeviceResources->GetCommandList();

    //NOTE(sean): This lets us render to a portion of the screen, we're not doing, but this is still mandatory.
    // Can be ignored.
    auto viewport = m_pDeviceResources->GetScreenViewport();
    auto scissorRect = m_pDeviceResources->GetScissorRect();

    m_pCommandList->RSSetViewports(1, &viewport);
    m_pCommandList->RSSetScissorRects(1, &scissorRect);
}

void Renderer::EndFrame() {
    //NOTE(sean): derived from some internal parberry code.
    if (m_screenShot) {
        BetterScreenShot();
        m_screenShot = false;
    }

    //NOTE(sean): Tell the GPU that we want to swap the Back Buffer with the Front Buffer.
    m_pDeviceResources->Present();

    //NOTE(sean): Upload commands to the GPU and tell it to execute them.
    m_pGraphicsMemory->Commit(m_pDeviceResources->GetCommandQueue());

    m_frameNumber += 1;
}

/// Begin rendering a debug frame.
/// Put all DrawDebugXYZ() or other functions in between this and EndDebugFrame()
void Renderer::BeginDebugDrawing() {
    //NOTE(sean): Debug frames are a bit different than our normal frames
    // because we only want to render the color to the output,
    // without any intermittent processing

    auto dsvDescBackBuffer = m_pDeviceResources->GetDepthStencilView();
    auto rtvDescBackBuffer = m_pDeviceResources->GetRenderTargetView();

    D3D12_CPU_DESCRIPTOR_HANDLE rtvDescs[1] = {
        rtvDescBackBuffer
    };

    m_pCommandList->OMSetRenderTargets(_countof(rtvDescs), rtvDescs, FALSE, &dsvDescBackBuffer);
    m_pCommandList->ClearDepthStencilView(dsvDescBackBuffer, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, 0);
}

/// Begin rendering a debug frame.
/// Put all DrawDebugXYZ() or other functions in between this and EndDebugFrame()
void Renderer::EndDebugDrawing() {
    //NOTE(sean): Stub for now
}

static Vec3 camera_saved_orient;
static Vec3 camera_saved_pos;
void Renderer::BeginUIDrawing() {
    ID3D12DescriptorHeap* pHeap[] = { m_pDescriptorHeap->Heap(), m_pStates->Heap() };
    m_pCommandList->SetDescriptorHeaps(_countof(pHeap), pHeap);

    auto viewport = m_pDeviceResources->GetScreenViewport();
    auto scissorRect = m_pDeviceResources->GetScissorRect();

    m_pCommandList->RSSetViewports(1, &viewport);
    m_pCommandList->RSSetScissorRects(1, &scissorRect);

    m_pSpriteBatch->Begin(m_pCommandList);

    auto dsvDescBackBuffer = m_pDeviceResources->GetDepthStencilView();
    auto rtvDescBackBuffer = m_pDeviceResources->GetRenderTargetView();

    D3D12_CPU_DESCRIPTOR_HANDLE rtvDescs[1] = {
        rtvDescBackBuffer
    };

    m_pCommandList->OMSetRenderTargets(_countof(rtvDescs), rtvDescs, FALSE, &dsvDescBackBuffer);
    m_pCommandList->ClearDepthStencilView(dsvDescBackBuffer, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, 0);

    m_uiRenderDepth = 1000.0f;

    //NOTE(sean): camera stuff
    const f32 width = (f32)m_nWinWidth;
    const f32 height = (f32)m_nWinHeight;

    const f32 near_clip = 0.1f;
    const f32 far_clip = 1000000.0f;
    
    camera_saved_orient = Vec3(m_pCamera->GetYaw(), m_pCamera->GetPitch(), m_pCamera->GetRoll());
    camera_saved_pos = m_pCamera->GetPos();
    m_pCamera->SetYaw(0.0f);
    m_pCamera->SetPitch(0.0f);
    m_pCamera->SetRoll(0.0f);
    m_pCamera->SetOrthographic(width, height, near_clip, far_clip);
    m_pCamera->MoveTo(Vec3(0.0f));
}

void Renderer::EndUIDrawing() {
    m_pSpriteBatch->End();

    //NOTE(sean): camera stuff
    const f32 width = (f32)m_nWinWidth;
    const f32 height = (f32)m_nWinHeight;
    const f32 aspect = width / height;

    //TODO(sean): Load this from settings
    const f32 fov = 90.0;
    const f32 fov_radians = (fov / 180.0) * XM_PI;

    const f32 near_clip = 0.1f;
    const f32 far_clip = 1000000.0f;
    
    m_pCamera->SetYaw(camera_saved_orient.x);
    m_pCamera->SetPitch(camera_saved_orient.y);
    m_pCamera->SetRoll(camera_saved_orient.z);
    m_pCamera->SetPerspective(aspect, fov_radians, near_clip, far_clip);
    m_pCamera->MoveTo(camera_saved_pos);
}

template <const u32 Start, const u32 End>
void clear_render_target_views(ID3D12GraphicsCommandList* command_list, DescriptorHeap* renders, RenderTexture* textures) {
    for every(index, End - Start + 1) {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = renders->GetCpuHandle(index + Start);
        Vec4 clear_color = textures[index + Start].m_clearColor;
        command_list->ClearRenderTargetView(handle, clear_color, 0, 0);
    }
}

template <const u32 Start, const u32 End>
void set_outputs(ID3D12GraphicsCommandList* command_list, DescriptorHeap* renders, RenderTexture* textures, D3D12_CPU_DESCRIPTOR_HANDLE* depth) {
    std::array<CD3DX12_CPU_DESCRIPTOR_HANDLE, End - Start + 1> descriptors;
    for every(index, End - Start + 1) {
        descriptors[index] = renders->GetCpuHandle(Start + index);
    }

    command_list->OMSetRenderTargets(End - Start + 1, descriptors.data(), FALSE, depth);

    for every(index, End - Start + 1) {
        textures[index + Start].TransitionTo(command_list, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }
}

template <const u32 Start, const u32 End>
void set_inputs(ID3D12GraphicsCommandList* command_list, RenderTexture* textures) {
    for every(index, End - Start + 1) {
        textures[index + Start].TransitionTo(command_list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
}

/// Begin rendering a frame.
/// Put all DrawXYZ() or other functions in between this and EndFrame()
void Renderer::BeginDrawing() {
    ID3D12DescriptorHeap* heap[] = { m_pDescriptorHeap->Heap() };
    m_pCommandList->SetDescriptorHeaps(1, heap);
    CD3DX12_CPU_DESCRIPTOR_HANDLE depth = m_pDeviceResources->GetDepthStencilView();
    set_outputs<Descriptors::DeferredColor, Descriptors::DeferredPosition>(m_pCommandList, m_pRendersHeap.get(), m_renderTextures.data(), &depth);
    clear_render_target_views<Descriptors::DeferredColor, Descriptors::DeferredPosition>(m_pCommandList, m_pRendersHeap.get(), m_renderTextures.data());
    m_pCommandList->ClearRenderTargetView(m_pDeviceResources->GetRenderTargetView(), m_f32BgColor, 0, 0);
    m_pCommandList->ClearDepthStencilView(depth, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, 0);
}

struct PPResourcesBundle {
    DescriptorHeap* resources;
    DescriptorHeap* renders;
    RenderTexture* textures;
};

template <typename E, const u32 ResStart, const u32 ResEnd, const u32 RenStart, const u32 RenEnd>
static inline void render_post_process(
    ID3D12GraphicsCommandList* command_list, 
    E* effect, 
    PPResourcesBundle bundle,
    D3D12_CPU_DESCRIPTOR_HANDLE* depth
) {
    set_inputs<ResStart, ResEnd>(command_list, bundle.textures);
    set_outputs<RenStart, RenEnd>(command_list, bundle.renders, bundle.textures, depth);
    clear_render_target_views<RenStart, RenEnd>(command_list, bundle.renders, bundle.textures);
    effect->SetTextures(bundle.resources->GetGpuHandle(ResStart));
    effect->Apply(command_list);
    command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    command_list->DrawInstanced(3, 1, 0, 0);
}

/// End Rendering a frame.
/// Put all DrawXYZ() or other functions in between this and BeginFrame()
void Renderer::EndDrawing() {
    //static Entity e = lights.Add({ Vec4(500.0f, 200.0f, 0.0f, 0.0f), Colors::CornflowerBlue * 800.0f });

    assert(lights.Size() < 254);
    m_lighting->SetLightCount(lights.Size());
    memcpy(m_lighting->Lights(), lights.Components(), sizeof(Light) * lights.Size()); 

    m_lighting->SetCameraPosition(m_pCamera->GetPos());

    ID3D12DescriptorHeap* pHeap[1] = { m_pResourcesHeap->Heap() };
    m_pCommandList->SetDescriptorHeaps(1, pHeap);

    PPResourcesBundle bundle = {};
    bundle.resources = m_pResourcesHeap.get();
    bundle.renders = m_pRendersHeap.get();
    bundle.textures = m_renderTextures.data();

    auto viewport = m_pDeviceResources->GetScreenViewport();
    auto scissorRect = m_pDeviceResources->GetScissorRect();

    m_bloomExtract->SetConstants(1.2);
    m_bloomBlur->SetConstants(0.03, 1.0);
    m_bloomCombine->SetConstants(1.0);

    render_post_process<
        LightingEffect, 
        Descriptors::DeferredColor, Descriptors::DeferredPosition,
        Descriptors::LightingColor, Descriptors::LightingColor
    >(m_pCommandList, m_lighting.get(), bundle, 0);

    // Render Bloom Extract Effect
    render_post_process<
        BloomExtractEffect,
        Descriptors::LightingColor, Descriptors::LightingColor,
        Descriptors::BloomExtract, Descriptors::BloomExtract
    >(m_pCommandList, m_bloomExtract.get(), bundle, 0);

    // Render Bloom Blur Effect
    render_post_process<
        BloomBlurEffect,
        Descriptors::BloomExtract, Descriptors::BloomExtract,
        Descriptors::BloomBlur0, Descriptors::BloomBlur0
    >(m_pCommandList, m_bloomBlur.get(), bundle, 0);

    viewport.Width /= 2; viewport.Height /= 2;
    m_pCommandList->RSSetViewports(1, &viewport);
    render_post_process<
        BloomBlurEffect,
        Descriptors::BloomBlur0, Descriptors::BloomBlur0,
        Descriptors::BloomBlur1, Descriptors::BloomBlur1
    >(m_pCommandList, m_bloomBlur.get(), bundle, 0);

    viewport.Width /= 2; viewport.Height /= 2;
    m_pCommandList->RSSetViewports(1, &viewport);
    render_post_process<
        BloomBlurEffect,
        Descriptors::BloomBlur1, Descriptors::BloomBlur1,
        Descriptors::BloomBlur2, Descriptors::BloomBlur2
    >(m_pCommandList, m_bloomBlur.get(), bundle, 0);

    viewport.Width /= 2; viewport.Height /= 2;
    m_pCommandList->RSSetViewports(1, &viewport);
    render_post_process<
        BloomBlurEffect,
        Descriptors::BloomBlur2, Descriptors::BloomBlur2,
        Descriptors::BloomBlur3, Descriptors::BloomBlur3
    >(m_pCommandList, m_bloomBlur.get(), bundle, 0);

    viewport.Width /= 2; viewport.Height /= 2;
    m_pCommandList->RSSetViewports(1, &viewport);
    m_bloomBlur->SetConstants(0.0009765625, 16.0);
    render_post_process<
        BloomBlurEffect,
        Descriptors::BloomBlur3, Descriptors::BloomBlur3,
        Descriptors::BloomBlur4, Descriptors::BloomBlur4
    >(m_pCommandList, m_bloomBlur.get(), bundle, 0);

    viewport.Width /= 2; viewport.Height /= 2;
    m_pCommandList->RSSetViewports(1, &viewport);
    render_post_process<
        BloomBlurEffect,
        Descriptors::BloomBlur4, Descriptors::BloomBlur4,
        Descriptors::BloomBlur5, Descriptors::BloomBlur5
    >(m_pCommandList, m_bloomBlur.get(), bundle, 0);

    // Render Bloom Combine Effect
    viewport.Width *= 4; viewport.Height *= 4;
    m_pCommandList->RSSetViewports(1, &viewport);
    render_post_process<
        BloomCombineEffect,
        Descriptors::BloomBlur4, Descriptors::BloomBlur5,
        Descriptors::BloomCombine3, Descriptors::BloomCombine3
    >(m_pCommandList, m_bloomCombine.get(), bundle, 0);

    viewport.Width *= 2; viewport.Height *= 2;
    m_pCommandList->RSSetViewports(1, &viewport);
    render_post_process<
        BloomCombineEffect,
        Descriptors::BloomBlur3, Descriptors::BloomCombine3,
        Descriptors::BloomCombine2, Descriptors::BloomCombine2
    >(m_pCommandList, m_bloomCombine.get(), bundle, 0);

    viewport.Width *= 2; viewport.Height *= 2;
    m_pCommandList->RSSetViewports(1, &viewport);
    render_post_process<
        BloomCombineEffect,
        Descriptors::BloomBlur2, Descriptors::BloomCombine2,
        Descriptors::BloomCombine1, Descriptors::BloomCombine1
    >(m_pCommandList, m_bloomCombine.get(), bundle, 0);

    viewport.Width *= 2; viewport.Height *= 2;
    m_pCommandList->RSSetViewports(1, &viewport);
    render_post_process<
        BloomCombineEffect,
        Descriptors::BloomBlur1, Descriptors::BloomCombine1,
        Descriptors::BloomCombine0, Descriptors::BloomCombine0
    >(m_pCommandList, m_bloomCombine.get(), bundle, 0);

    m_bloomCombine->SetConstants(0.4);
    render_post_process<
        BloomCombineEffect,
        Descriptors::LightingColor, Descriptors::BloomCombine0,
        Descriptors::BloomOutput, Descriptors::BloomOutput
    >(m_pCommandList, m_bloomCombine.get(), bundle, 0);

    // Render Tonemap Effect
    {
        //NOTE(sean): This is an identical process to the previous block of code, so check that for details
        set_inputs<Descriptors::BloomOutput, Descriptors::BloomOutput>(m_pCommandList, bundle.textures);

        m_pCommandList->OMSetRenderTargets(1, &m_pDeviceResources->GetRenderTargetView(), FALSE, 0);

        m_pTonemapEffect->SetTextures(m_pResourcesHeap->GetGpuHandle(Descriptors::BloomOutput));
        m_pTonemapEffect->UpdateConstants(m_nWinWidth, m_nWinHeight, tint_color, blur_amount, saturation_amount);

        m_pTonemapEffect->Apply(m_pCommandList);
        m_pCommandList->DrawInstanced(3, 1, 0, 0);
    }
}

/// Begins a debug line batch.
/// Code that follows this will be batched into ONE draw call
/// Needs to use compatable functions
void Renderer::BeginDebugLineBatch() {
    m_pPrimitiveBatch->Begin(m_pCommandList);

    XMMATRIX world = XMMatrixTransformation( g_XMZero, Quaternion::Identity, g_XMOne, g_XMZero, Quaternion::Identity, g_XMZero);

    m_pDebugLineEffect->SetWorld(world);
    m_pDebugLineEffect->SetView(XMLoadFloat4x4(&m_view));
    m_pDebugLineEffect->Apply(m_pCommandList);
}

/// Begins a debug triangle batch.
/// Code that follows this will be batched into ONE draw call
/// Needs to use compatable functions
void Renderer::BeginDebugTriangleBatch() {
    m_pPrimitiveBatch->Begin(m_pCommandList);

    m_pDebugTriangleEffect->SetView(XMLoadFloat4x4(&m_view));
    m_pDebugTriangleEffect->Apply(m_pCommandList);
}

/// Ends any currently open batch
/// Use this to "draw" the current batched items, however actual rendering is technically performed later
void Renderer::EndDebugBatch() {
    m_pPrimitiveBatch->End();
}

/// Draw a colored "Debug Line" from A to B
/// A - B
void Renderer::DrawDebugLine(
    const Vec3 A,
    const Vec3 B,
    const Vec4 color
) {
    m_pPrimitiveBatch->DrawLine(VertexPC(A, color), VertexPC(B, color));
}

/// Draw a colored "Debug Triangle".
///   A
///  / \
/// C - B
void Renderer::DrawDebugTriangle(
    const Vec3 A,
    const Vec3 B,
    const Vec3 C,
    const Vec4 color
) {
    VertexPC vertices[4] = { VertexPC(A, color), VertexPC(B, color), VertexPC(C, color), VertexPC(A, color) };
    m_pPrimitiveBatch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, vertices, 4);
}

/// Draw a colored "Debug Quad".
/// A - B
/// |   |
/// D - C
void Renderer::DrawDebugQuad(
    const Vec3 A,
    const Vec3 B,
    const Vec3 C,
    const Vec3 D,
    const Vec4 color
) {
    VertexPC vertices[5] = { VertexPC(A, color), VertexPC(B, color), VertexPC(C, color), VertexPC(D, color), VertexPC(A, color) };
    m_pPrimitiveBatch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, vertices, 5);
}

/// Draw a colored "Debug Ray".
/// A --->

void Renderer::DrawDebugRay(
    const Vec3 origin,
    const Vec3 direction,
    const f32 length,
    const Vec4 color
) {
    const Vector3 end = origin + direction * length;
    DrawDebugLine(origin, end, color);
}

/// Draw a colored "Debug Ring".
///      A - B
///    /       \
///   H         C
///   |         |
///   G         D
///    \       /
///      F - E
void Renderer::DrawDebugRing(
    const Vec3 origin,
    const Vec3 orientation,
    const f32 radius,
    const u32 segments,
    const Vec4 color
) {
    // select a normal
    Vector3 normal(orientation.z, orientation.z, -orientation.x-orientation.y);
    if (normal.x == 0 && normal.y == 0 && normal.z == 0) {
        normal = Vector3(-orientation.y - orientation.z, orientation.x, orientation.x);
    }

    DrawDebugRay(origin, orientation, radius, Colors::LightPink);
    DrawDebugRay(origin, normal, radius, Colors::LightPink);
    //DrawDebugRay(origin, normal2, radius, Colors::LightPink);
}

//NOTE(sean): https://github.com/Microsoft/DirectXTK/wiki/DebugDraw
//TODO(sean): If performance ***really*** becomes a problem, make this nocopy
/// Draw a colored "Debug Ring".
/// This does the same thing as DrawDebugRing()
void Renderer::DrawDebugRing2(
    Vec3 origin,
    Vec3 majorAxis,
    Vec3 minorAxis,
    const u32 segments,
    const Vec4 color
) {
    VertexPC* verts = (VertexPC*)m_debugScratch.Alloc(((usize)segments + 1) * sizeof(VertexPC));

    FLOAT fAngleDelta = XM_2PI / float(segments);
    // Instead of calling cos/sin for each segment we calculate
    // the sign of the angle delta and then incrementally calculate sin
    // and cosine from then on.
    XMVECTOR cosDelta = XMVectorReplicate(cosf(fAngleDelta));
    XMVECTOR sinDelta = XMVectorReplicate(sinf(fAngleDelta));
    XMVECTOR incrementalSin = XMVectorZero();
    static const XMVECTORF32 s_initialCos =
    {
        { { 1.f, 1.f, 1.f, 1.f } }
    };
    XMVECTOR incrementalCos = s_initialCos.v;
    for (size_t i = 0; i < segments; i++)
    {
        XMVECTOR pos = XMVectorMultiplyAdd(majorAxis, incrementalCos, origin);
        pos = XMVectorMultiplyAdd(minorAxis, incrementalSin, pos);
        XMStoreFloat3(&verts[i].position, pos);
        XMStoreFloat4(&verts[i].color, color);
        // Standard formula to rotate a vector.
        XMVECTOR newCos = XMVectorSubtract(XMVectorMultiply(incrementalCos, cosDelta), XMVectorMultiply(incrementalSin, sinDelta));
        XMVECTOR newSin = XMVectorAdd(XMVectorMultiply(incrementalCos, sinDelta), XMVectorMultiply(incrementalSin, cosDelta));
        incrementalCos = newCos;
        incrementalSin = newSin;
    }
    verts[segments] = verts[0];

    m_pPrimitiveBatch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, (usize)segments + 1);

    m_debugScratch.Reset(0);
}

/// Draw a colored "Debug Sphere".
/// I'm not drawing a diagram for this one
void Renderer::DrawDebugSphere(
    const BoundingSphere sphere,
    const u32 segments,
    const Vec4 color
) {
    XMVECTOR center = XMLoadFloat3(&sphere.Center);

    XMVECTOR x_axis = XMVectorScale(g_XMIdentityR0, sphere.Radius);
    XMVECTOR y_axis = XMVectorScale(g_XMIdentityR1, sphere.Radius);
    XMVECTOR z_axis = XMVectorScale(g_XMIdentityR2, sphere.Radius);
    DrawDebugRing2(center, x_axis, z_axis, segments, color);
    DrawDebugRing2(center, x_axis, y_axis, segments, color);
    DrawDebugRing2(center, y_axis, z_axis, segments, color);
}

//NOTE(sean): https://github.com/Microsoft/DirectXTK/wiki/DebugDraw
/// Draw a colored box
void Renderer::DrawDebugAABB(
    const BoundingBox box,
    const Vec4 color
) {
    XMMATRIX world = MoveScaleMatrix(box.Center, box.Extents);
    DrawDebugCubeInternal(world, color);
}

//NOTE(sean): https://github.com/Microsoft/DirectXTK/wiki/DebugDraw
/// Draw a colored box, but it's rotated
void Renderer::DrawDebugOBB(
    const BoundingOrientedBox obb,
    const Vec4 color
) {
    XMMATRIX world = MoveRotateScaleMatrix(obb.Center, obb.Orientation, obb.Extents);
    DrawDebugCubeInternal(world, color);
}

/// Draw a colored "Debug Capsule"
void Renderer::DrawDebugCapsule(
   const Vec3 origin, 
   const f32 radius, 
   const f32 height, 
   const u32 segments,
   const Vec4 color
) {
    XMVECTOR center = XMLoadFloat3(&origin);

    XMVECTOR x_axis = XMVectorScale(g_XMIdentityR0, radius);
    XMVECTOR y_axis = XMVectorScale(g_XMIdentityR1, radius);
    XMVECTOR z_axis = XMVectorScale(g_XMIdentityR2, radius);
    DrawDebugRing2(XMVectorSetY(center, XMVectorGetY(center) - height), x_axis, z_axis, segments, color);
    DrawDebugRing2(XMVectorSetY(center, XMVectorGetY(center) + height), x_axis, z_axis, segments, color);
    DrawDebugRing2(XMVectorSetY(center, XMVectorGetY(center) - height), x_axis, y_axis, segments, color);
    DrawDebugRing2(XMVectorSetY(center, XMVectorGetY(center) + height), x_axis, y_axis, segments, color);
    DrawDebugRing2(XMVectorSetY(center, XMVectorGetY(center) - height), y_axis, z_axis, segments, color);
    DrawDebugRing2(XMVectorSetY(center, XMVectorGetY(center) + height), y_axis, z_axis, segments, color);

    for every(point, 4) {
        XMVECTOR x = { origin.x + radius * cosf((2 * XM_PI) * point / 4), origin.y + height, origin.z + radius * sinf((2 * XM_PI) * point / 4) };
        XMVECTOR y = XMVectorSetY(x, XMVectorGetY(x) - height * 2);
        m_pPrimitiveBatch->DrawLine(VertexPC(x, color), VertexPC(y, color));
    }

}

//NOTE(sean): https://github.com/Microsoft/DirectXTK/wiki/DebugDraw
// I tried to make this faster but it turns out this is really damn fast.
// The only way I could feasibly make this faster would be by mapping the internal buffer myself.
// This would get rid of the deep copy the DrawXYZ() functions perform
/// Draw a colored "Debug Grid"
/// A - B - C
/// |   |   |
/// D - E - F
/// |   |   |
/// G - H - I
void Renderer::DrawDebugGrid(
    const Vec3 x_axis,
    const Vec3 y_axis,
    const Vec3 origin,
    const u32 x_segments,
    const u32 y_segments,
    const Vec4 color
) {
    for every(i, x_segments) {
        f32 percent = f32(i) / f32(x_segments - 1);
        percent = (percent * 2.f) - 1.f;
        XMVECTOR scale = XMVectorScale(x_axis, percent);
        scale = XMVectorAdd(scale, origin);

        Vector3 v1(XMVectorSubtract(scale, y_axis));
        Vector3 v2(XMVectorAdd(scale, y_axis));

        m_pPrimitiveBatch->DrawLine(VertexPC(v1, color), VertexPC(v2, color));
    }

    for every(i, y_segments) {
        f32 percent = f32(i) / f32(y_segments - 1);
        percent = (percent * 2.f) - 1.f;
        XMVECTOR scale = XMVectorScale(y_axis, percent);
        scale = XMVectorAdd(scale, origin);

        Vector3 v1(XMVectorSubtract(scale, x_axis));
        Vector3 v2(XMVectorAdd(scale, x_axis));

        m_pPrimitiveBatch->DrawLine(VertexPC(v1, color), VertexPC(v2, color));
    }
}

//NOTE(sean): https://github.com/Microsoft/DirectXTK/wiki/DebugDraw
/// Internal function (ask sean if you want to know what it does)
void Renderer::DrawDebugCubeInternal(
    const Mat4x4& world,
    const Vec4 color
) {
     const XMVECTORF32 vert_pos[8] = {
         {{{ -1.f, -1.f, -1.f, 0.f }}},
         {{{  1.f, -1.f, -1.f, 0.f }}},
         {{{  1.f, -1.f,  1.f, 0.f }}},
         {{{ -1.f, -1.f,  1.f, 0.f }}},
         {{{ -1.f,  1.f, -1.f, 0.f }}},
         {{{  1.f,  1.f, -1.f, 0.f }}},
         {{{  1.f,  1.f,  1.f, 0.f }}},
         {{{ -1.f,  1.f,  1.f, 0.f }}}
     };

    const u16 indices[24] = {
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        4, 5,
        5, 6,
        6, 7,
        7, 4,
        0, 4,
        1, 5,
        2, 6,
        3, 7
    };

    VertexPC vertices[8];
    for every(i, 8) {
        XMVECTOR v = XMVector3Transform(vert_pos[i], world);
        XMStoreFloat3(&vertices[i].position, v);
        XMStoreFloat4(&vertices[i].color, color);
    }

    m_pPrimitiveBatch->DrawIndexed(D3D_PRIMITIVE_TOPOLOGY_LINELIST, indices, 24, vertices, 8);
}

/// Add a DebugModel to the renderer's internal list.
/// This function will return a handle to the index in the list.
u32 Renderer::AddDebugModel(DebugModel* model) {
    m_debugModels.push_back(*model);
    return (u32)m_debugModels.size() - 1;
}

/// Draw an instance of a DebugModel.
/// For example, you might want to render 3 enemies of the same type.
/// Instead of storing an entire copy of the model in each enemy, we store the model once,
/// then reference it with a "handle", in this case a bog-standard index into an array.
/// To move, scasle, and rotate the model, change the world matrix in the instance.
void Renderer::DrawDebugModelInstance(ModelInstance* instance) {
    DebugModel* pModel = &m_debugModels[instance->model];

    switch (pModel->m_modelType) {
    case(DebugModelType::LINE_LIST): {
        BeginDebugLineBatch();
        {
            m_pDebugLineEffect->SetWorld(instance->world);
            m_pDebugLineEffect->Apply(m_pCommandList);
            DrawDebugLineModel(pModel);
        }
        EndDebugBatch();
    } break;

    case(DebugModelType::TRIANGLE_LIST): {
        BeginDebugTriangleBatch();
        {
            m_pDebugTriangleEffect->SetWorld(instance->world);
            m_pDebugTriangleEffect->Apply(m_pCommandList);
            DrawDebugTriangleModel(pModel);
        }
        EndDebugBatch();
    } break;
    }
}

/// Resets the debug world matricies to zero, you probably dont need to touch this though.
void Renderer::ResetDebugWorldMatrix() {
    //NOTE(sean): we might want to look into not making this happen on every draw call
    const XMMATRIX world = XMMatrixIdentity();
    m_pDebugLineEffect->SetWorld(world);
    m_pDebugLineEffect->Apply(m_pCommandList);
}

//NOTE(sean): if performance is required, this can be made nocopy
/// Internal method to draw a DebugModel
void Renderer::DrawDebugLineModel(DebugModel* model) {
    m_pPrimitiveBatch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINELIST, model->m_vertexList.data(), model->m_vertexList.size());
}

/// Internal method to draw a DebugModel
void Renderer::DrawDebugTriangleModel(DebugModel* model) {
    m_pPrimitiveBatch->Draw(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, model->m_vertexList.data(), model->m_vertexList.size());
}

/// Get the number of frames elapsed since the start of the program
const usize Renderer::GetNumFrames() const {
    return m_frameNumber;
}


/// Get the windows Hwnd (Window Handle)
HWND Renderer::GetHwnd() {
    return m_Hwnd;
}

/// Get the horizontal resolution
u32 Renderer::GetResolutionWidth() {
    return m_nWinWidth;
}

/// Get the vertical resolution
u32 Renderer::GetResolutionHeight() {
    return m_nWinHeight;
}

//NOTE(sean): VBO file format: https://github.com/microsoft/DirectXMesh/blob/master/Meshconvert/Mesh.cpp
struct VBOData {
    u32 vertex_count;
    u32 index_count;
    VertexPNT* vertices;
    u16* indices;

    void VBOData::free() {
        delete[] vertices;
        delete[] indices;
    }
};

//NOTE(sean): VBO file format: https://github.com/microsoft/DirectXMesh/blob/master/Meshconvert/Mesh.cpp
void LoadVBO(const char* fpath, VBOData* model) {
    #define CHECK(value) ABORT_EQ_FORMAT((value), 0, "Corrupt VBO file!")

    FILE* fp = fopen(fpath, "rb"); CHECK(fp);

    CHECK(fread(&model->vertex_count, sizeof(u32), 1, fp));
    CHECK(fread(&model->index_count, sizeof(u32), 1, fp));

    model->vertices = new VertexPNT[model->vertex_count]; CHECK(model->vertices); 
    model->indices = new u16[model->index_count]; CHECK(model->indices);

    CHECK(fread(model->vertices, sizeof(VertexPNT), model->vertex_count, fp));
    CHECK(fread(model->indices, sizeof(u16), model->index_count, fp));

    fclose(fp);
}

template <typename T>
inline void Renderer::CreateBufferAndView(T* data, usize count, GraphicsResource& resource, std::shared_ptr<D3D12_VERTEX_BUFFER_VIEW>& view) {
    isize size = sizeof(T) * count;

    resource = GraphicsMemory::Get().Allocate(size);
    memcpy(resource.Memory(), data, size); // i like this function

    view = std::make_shared<D3D12_VERTEX_BUFFER_VIEW>();
    view->BufferLocation = resource.GpuAddress();
    view->StrideInBytes = sizeof(T);
    view->SizeInBytes = (u32)resource.Size();
}

template <typename T>
inline void Renderer::CreateBufferAndView(T* data, usize count, GraphicsResource& resource, std::shared_ptr<D3D12_INDEX_BUFFER_VIEW>& view) {
    isize size = sizeof(T) * count;

    resource = GraphicsMemory::Get().Allocate(size);
    memcpy(resource.Memory(), data, size);
    
    view = std::make_shared<D3D12_INDEX_BUFFER_VIEW>();
    view->BufferLocation = resource.GpuAddress();
    view->SizeInBytes = (u32)resource.Size();
    view->Format = DXGI_FORMAT_R32_UINT;
}

/// Load a debug model with the name.
/// Debug models are assumed to be VBO (.vbo) files.
u32 Renderer::LoadDebugModel(const char* name, Vec4 color) {
    std::string fpath = XMLFindItem(m_pXmlSettings, "models", "model", name);
    ABORT_EQ_FORMAT(fpath, "", "Unable to find \"%s\\%s\\%s\"", "models", "model", name);

    VBOData vbo_data;
    LoadVBO(fpath.c_str(), &vbo_data);

    //NOTE(sean): closest number under 2048 that is a multiple of 3,
    //this seems to be the limit for the current primitive batch
    if (vbo_data.index_count > 2046) {
        ABORT("Input VBO must not contain more than 2046 indices!");
    }

    VertexPC* mesh = new VertexPC[vbo_data.index_count];

    // unpack index list
    for every(index, vbo_data.index_count) {
        mesh[index] = { vbo_data.vertices[vbo_data.indices[index]].position, *(XMFLOAT4*)&color }; // position and color
    }

    u32 handle = AddDebugModel(&DebugModel(mesh, vbo_data.index_count, DebugModelType::LINE_LIST));

    vbo_data.free();
    delete[] mesh;

    return handle;
}

/// Load a model.
/// Models are assumed to be VBO (.vbo) files.
void Renderer::LoadModel(const char* name, u32 model_index) {
    if (model_index > m_models.size()) { ABORT("Model index exceeds total number of models!"); }

    std::string fpath = XMLFindItem(m_pXmlSettings, "models", "model", name);
    ABORT_EQ_FORMAT(fpath, "", "Unable to find \"%s\\%s\\%s\"", "models", "model", name);

    VBOData vbo_data;
    LoadVBO(fpath.c_str(), &vbo_data);

    GameModel* pmodel = &m_models[model_index];
    pmodel->index_count = vbo_data.index_count;

    u32* indices = new u32[vbo_data.index_count];
    for every(index, vbo_data.index_count) {
        indices[index] = vbo_data.indices[index];
    }

    CreateBufferAndView<VertexPNT>(vbo_data.vertices, vbo_data.vertex_count, pmodel->vertex_buffer, pmodel->vertex_view);
    CreateBufferAndView<u32>(indices, vbo_data.index_count, pmodel->index_buffer, pmodel->index_view);

    vbo_data.free();
    delete[] indices;
}

/// Load a texture.
/// Textures are assumed to be DDS (.dds) files.
void Renderer::LoadTextureI(const char* name, u32 texture_index) {

    std::string fpath = XMLFindItem(m_pXmlSettings, "textures", "texture", name);
    ABORT_EQ_FORMAT(fpath, "", "Unable to find \"%s\\%s\\%s\"", "textures", "texture", name);

    LTextureDesc d;
    LoadTextureFile(fpath.c_str(), d);
}

void Renderer::DrawModelInstance(ModelInstance* instance) {
    //TODO(sean): check if this can be moved out when we finalize the debug and game drawing APIs
    m_deferred->SetWorld(instance->world);
    m_deferred->SetView(XMLoadFloat4x4(&m_view));
    m_deferred->SetTextures(m_pDescriptorHeap->GetGpuHandle(instance->texture));
    m_deferred->SetGlow(instance->glow);

    m_deferred->Apply(m_pCommandList);

    GameModel* pmodel = &m_models[instance->model];
    m_pCommandList->IASetVertexBuffers(0, 1, pmodel->vertex_view.get());
    m_pCommandList->IASetIndexBuffer(pmodel->index_view.get());

    m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_pCommandList->DrawIndexedInstanced(pmodel->index_count, 1, 0, 0, 0);
}

/*
void Renderer::DrawParticleInstance(ParticleInstance* instance) {
    //TODO(sean): check if this can be moved out when we finalize the debug and game drawing APIs
    ModelInstance* pmi = &instance->particle_instance;

    Ecs::ApplyEvery(instance->particles, [=](Entity e) {
        Particle* particle = m_Particles.Get(e);

        pmi->world = MoveScaleMatrix(particle->pos, instance->model_scale);
        m_deferred->SetWorld(pmi->world);

        m_deferred->SetView(XMLoadFloat4x4(&m_view));
        m_deferred->SetTextures(m_pDescriptorHeap->GetGpuHandle(TextureIndex::White));

        m_deferred->SetGlow(instance->glow);

        m_deferred->Apply(m_pCommandList);

        GameModel* pmodel = &m_models[instance->particle_instance.model];
        m_pCommandList->IASetVertexBuffers(0, 1, pmodel->vertex_view.get());
        m_pCommandList->IASetIndexBuffer(pmodel->index_view.get());

        m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        m_pCommandList->DrawIndexedInstanced(pmodel->index_count, 1, 0, 0, 0);
    });
}
*/

//ParticleInstance Renderer::CreateParticleInstance(ParticleInstanceDesc* desc) {
//    ParticleInstance instance;
//    instance.light = lights.Add({ *(Vec4*)&desc->initial_pos, *(Vec4*)&desc->light_color });
//    instance.model_scale = desc->size;
//
//    instance.particle_instance.model = desc->model;
//    instance.particle_instance.texture = desc->texture;
//
//    instance.glow = desc->glow;
//
//    u32 count = GameRandom::Randu32(desc->min_count, desc->max_count);
//
//    for every(index, count) {
//        Entity e = Entity();
//
//        Particle particle;
//        particle.pos = desc->initial_pos;
//
//        Vec3 dir = JitterVec3(desc->initial_dir, -2.0f * desc->dir_randomness, 2.0f * desc->dir_randomness);
//        particle.vel = desc->initial_speed * dir;
//
//        Vec3 acc = JitterVec3(desc->initial_acc, -2.0f * desc->acc_randomness, 2.0f * desc->acc_randomness);
//        particle.acc = acc;
//
//        instance.particles.AddExisting(e);
//    }
//
//    return instance;
//}

void Renderer::Update() {
    //f32 dt = m_pTimer->GetFrameTime();

    //for every(index, m_ParticleTimers.Size()) {
    //    m_ParticleTimers.Components()[index] -= dt;
    //}

    //Ecs::ApplyEvery(m_ParticlesActive, [&](Entity e) {
    //    Particle* p = m_Particles.Get(e);

    //    p->vel += p->acc * dt;
    //    p->pos += p->vel * dt;
    //});

    //Ecs::RemoveConditionally(
    //    m_ParticlesActive,
    //    [&](Entity e) { return *m_ParticleTimers.Get(e) <= 0.0f; },
    //    [&](Entity e) {}
    //);
}

// Sean: this is an adaptation of LSpriteRenderer::Draw(LSpriteDesc2D*)'s Batched2D mode
void Renderer::DrawSpriteInstance(SpriteInstance* instance) {
    LBaseCamera* cam = m_pCamera;

    // Sean: the scalings are very random feeling

    Vec3 scale = Vec3(instance->scale.x, instance->scale.y, 1.0f);
    scale *= 1.7f;
    scale *= (m_uiRenderDepth / 1000.0f);

    // Sean: I'm really going to do some 3d rotation stuff because I can't be bothered to do the code
    // to properly set up a UI camera and a NORMAL camera.
    const Vec2 pos = Vec2(instance->position.x, -instance->position.y) * 1.75f;
    const Vec2 win_scl = Vec2(-888.0f, 668.0f);
    Vec2 real_pos = pos + win_scl + Vec2(scale.x / 2.0f, -scale.y / 2.0f);
    real_pos *= (m_uiRenderDepth / 1000.0f);

    const Quat orientation = Quaternion::CreateFromYawPitchRoll(0.0f, 0.0f, instance->roll);

    const Mat4x4 world = MoveRotateScaleMatrix(Vec3(real_pos.x, real_pos.y, m_uiRenderDepth), orientation, scale);

    m_uiRenderDepth -= 0.2f;

    m_spriteEffect->SetTexture(
        m_pDescriptorHeap->GetGpuHandle(instance->texture_index),
        m_pStates->AnisotropicClamp()
    );

    m_spriteEffect->SetColorAndAlpha(instance->rgba);

    m_spriteEffect->SetWorld(world);
    m_spriteEffect->SetView(XMLoadFloat4x4(&m_view));

    m_spriteEffect->Apply(m_pCommandList);

    m_pCommandList->IASetVertexBuffers(0, 1, m_spriteVertexBufferView.get());
    m_pCommandList->IASetIndexBuffer(m_spriteIndexBufferView.get());
    m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    m_pCommandList->DrawIndexedInstanced(4, 1, 0, 0, 0);
}
