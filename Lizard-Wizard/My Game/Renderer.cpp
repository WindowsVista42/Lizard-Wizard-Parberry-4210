#include "Renderer.h"
#include "Math.h"
#include "Helpers.h"
#include <iostream>
#include "Keyboard.h"
#include "GraphicsHelpers.h"

Renderer::Renderer():
    LRenderer3D(),
    m_pCamera(new LBaseCamera),
    m_debugScratch(16 * 1024), // 16k
    m_models((u32)ModelType::Count)
{
    //NOTE(sean): Windows window stuff
    m_f32BgColor = Colors::Black; // NOTE(sean): set the clear color
    ShowCursor(0); // NOTE(sean): win32 hide the cursor

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
    m_pCamera->MoveTo(Vector3(0.0f, 0.0f, 0.0f));

}

Renderer::~Renderer() {
    delete m_pCamera;
    m_pDeviceResources->WaitForGpu();
}

void Renderer::Initialize() {
    LRenderer3D::Initialize(true);

    //TODO(sean): resource cleanup
    //NOTE(sean): Deferred Effect
    {
        CreateRenderTextureHeaps(
            m_pD3DDevice,
            &m_pDeferredResourceDescs,
            &m_pDeferredRenderDescs,
            DeferredPass::Count
        );

        m_deferredPassTextures.resize(DeferredPass::Count);
        m_deferredPassTextures[DeferredPass::Color] = RenderTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black);
        m_deferredPassTextures[DeferredPass::Normal] = RenderTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black);
        m_deferredPassTextures[DeferredPass::Position] = RenderTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black);

        m_deferredPassTextures[DeferredPass::Color].Init(
            m_pD3DDevice, 
            m_pDeferredResourceDescs->GetCpuHandle(DeferredPass::Color),
            m_pDeferredRenderDescs->GetCpuHandle(DeferredPass::Color),
            m_nWinWidth,
            m_nWinHeight
        );

        m_deferredPassTextures[DeferredPass::Normal].Init(
            m_pD3DDevice, 
            m_pDeferredResourceDescs->GetCpuHandle(DeferredPass::Normal),
            m_pDeferredRenderDescs->GetCpuHandle(DeferredPass::Normal),
            m_nWinWidth,
            m_nWinHeight
        );

        m_deferredPassTextures[DeferredPass::Position].Init(
            m_pD3DDevice, 
            m_pDeferredResourceDescs->GetCpuHandle(DeferredPass::Position),
            m_pDeferredRenderDescs->GetCpuHandle(DeferredPass::Position),
            m_nWinWidth,
            m_nWinHeight
        );
    }

    //NOTE(sean): Tonemap Effect
    {
        CreateRenderTextureHeaps(
            m_pD3DDevice,
            &m_pTonemapResourceDescs,
            &m_pTonemapRenderDescs,
            TonemapPass::Count
        );

        m_tonemapPassTextures.resize(TonemapPass::Count);
        m_tonemapPassTextures[TonemapPass::Color] = RenderTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black);

        m_tonemapPassTextures[TonemapPass::Color].Init(
            m_pD3DDevice,
            m_pTonemapResourceDescs->GetCpuHandle(TonemapPass::Color),
            m_pTonemapRenderDescs->GetCpuHandle(TonemapPass::Color),
            m_nWinWidth,
            m_nWinHeight
        );
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
        render_target_state.numRenderTargets = DeferredPass::Count;
        render_target_state.rtvFormats[DeferredPass::Color] = m_deferredPassTextures[DeferredPass::Color].m_format;
        render_target_state.rtvFormats[DeferredPass::Normal] = m_deferredPassTextures[DeferredPass::Normal].m_format;
        render_target_state.rtvFormats[DeferredPass::Position] = m_deferredPassTextures[DeferredPass::Position].m_format;
        render_target_state.sampleMask = ~0u;
        render_target_state.sampleDesc.Count = 1;

        EffectPipelineStateDescription pipeline_state_desc(
            &VertexPNT::InputLayout,
            CommonStates::Opaque,
            CommonStates::DepthDefault,
            CommonStates::CullCounterClockwise,
            render_target_state
        );

        m_pDeferredEffect = std::make_unique<DeferredEffect>(m_pD3DDevice, pipeline_state_desc);
        m_pDeferredEffect->SetProjection(XMLoadFloat4x4(&m_projection));
    }

    {
        RenderTargetState render_target_state = {};
        render_target_state.dsvFormat = m_pDeviceResources->GetDepthBufferFormat();
        render_target_state.numRenderTargets = TonemapPass::Count;
        render_target_state.rtvFormats[TonemapPass::Color] = m_tonemapPassTextures[TonemapPass::Color].m_format;
        render_target_state.sampleMask = ~0u;
        render_target_state.sampleDesc.Count = 1;

        EffectPipelineStateDescription pipeline_state_desc(
            0,
            CommonStates::Opaque,
            CommonStates::DepthNone,
            CommonStates::CullNone,
            render_target_state
        );

        m_pLightingEffect = std::make_unique<LightingEffect>(m_pD3DDevice, pipeline_state_desc);
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
    }
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
    //NOTE(sean): some internal parberry code, I don't know what it does,
    // but it just saves a screenshot and should work despite all of my changes.
    if (m_screenShot) {
        SaveScreenShot();
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

    //ID3D12DescriptorHeap* pHeaps[] = {};
    //m_pCommandList->SetDescriptorHeaps(_countof(pHeaps), pHeaps);

    auto dsvDescBackBuffer = m_pDeviceResources->GetDepthStencilView();
    auto rtvDescBackBuffer = m_pDeviceResources->GetRenderTargetView();

    D3D12_CPU_DESCRIPTOR_HANDLE rtvDescs[1] = {
        rtvDescBackBuffer
    };

    m_pCommandList->OMSetRenderTargets(_countof(rtvDescs), rtvDescs, FALSE, &dsvDescBackBuffer);

    //NOTE(sean): we would normally clear here, but we don't want to do that,
    // because we want the debug lines to be rendered-over like normal, and
    // we want a proper depth test
}

/// Begin rendering a debug frame.
/// Put all DrawDebugXYZ() or other functions in between this and EndDebugFrame()
void Renderer::EndDebugDrawing() {
    //NOTE(sean): Unless we need to do some actual post-processing,
    // this is a stub for now
}

/// Begin rendering a frame.
/// Put all DrawXYZ() or other functions in between this and EndFrame()
void Renderer::BeginDrawing() {
    ID3D12DescriptorHeap* pHeaps[] = {
        m_pDeferredResourceDescs->Heap(),
    };

    //NOTE(sean): Set descriptors so DirectX knows where to look to find our data
    m_pCommandList->SetDescriptorHeaps(_countof(pHeaps), pHeaps);

    auto dsvDescBackBuffer = m_pDeviceResources->GetDepthStencilView();

    auto rtvDescDiffuse = m_pDeferredRenderDescs->GetCpuHandle(DeferredPass::Color);
    auto diffuseClearColor = m_deferredPassTextures[DeferredPass::Color].m_clearColor;

    auto rtvDescNormal = m_pDeferredRenderDescs->GetCpuHandle(DeferredPass::Normal);
    auto normalClearColor = m_deferredPassTextures[DeferredPass::Normal].m_clearColor;

    auto rtvDescPosition = m_pDeferredRenderDescs->GetCpuHandle(DeferredPass::Position);
    auto positionClearColor = m_deferredPassTextures[DeferredPass::Position].m_clearColor;

    D3D12_CPU_DESCRIPTOR_HANDLE rtvDescs[DeferredPass::Count] = {
        rtvDescDiffuse,
        rtvDescNormal,
        rtvDescPosition
    };

    //NOTE(sean): Tell the gpu where we want to render, ie a "target"
    m_pCommandList->OMSetRenderTargets(DeferredPass::Count, rtvDescs, FALSE, &dsvDescBackBuffer);

    //NOTE(sean): Tell the gpu to clear the targets
    m_pCommandList->ClearRenderTargetView(rtvDescDiffuse, diffuseClearColor, 0, 0);
    m_pCommandList->ClearRenderTargetView(rtvDescNormal, normalClearColor, 0, 0);
    m_pCommandList->ClearRenderTargetView(rtvDescPosition, positionClearColor, 0, 0);
    m_pCommandList->ClearDepthStencilView(dsvDescBackBuffer, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, 0);
}

/// End Rendering a frame.
/// Put all DrawXYZ() or other functions in between this and BeginFrame()
void Renderer::EndDrawing() {
    // Render Lighting Effect
    {
    	//NOTE(sean): transition resources into the correct "read-only" state.
        // The code will still technically run without these, but the gpu wont know
        // that we want to wait until we're done writing to this.
        // In short, we get a data race.
    	m_deferredPassTextures[DeferredPass::Color].PushTransition(m_pCommandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    	m_deferredPassTextures[DeferredPass::Normal].PushTransition(m_pCommandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    	m_deferredPassTextures[DeferredPass::Position].PushTransition(m_pCommandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        //NOTE(sean): more of the same, we set the target
        auto rtvDescriptorColor = m_pTonemapRenderDescs->GetCpuHandle(TonemapPass::Color);
        m_pCommandList->OMSetRenderTargets(1, &rtvDescriptorColor, FALSE, 0);

        //NOTE(sean): Set textures to read from.
        // In the shader, these get sample from so we can properly build the output image.
        m_pLightingEffect->SetTextures(
            m_pDeferredResourceDescs->GetGpuHandle(DeferredPass::Color),
            m_pDeferredResourceDescs->GetGpuHandle(DeferredPass::Normal),
            m_pDeferredResourceDescs->GetGpuHandle(DeferredPass::Position)
        );

        //NOTE(sean): This actally "applies" the effect, any subsequent draw calls will use this effect
        m_pLightingEffect->Apply(m_pCommandList);

        //NOTE(sean): Draw effect
        m_pCommandList->DrawInstanced(3, 1, 0, 0);

        //NOTE(sean): Transition resources back into their original state,
        // allowing them to be written to again.
        m_deferredPassTextures[DeferredPass::Color].PopTransition(m_pCommandList);
    	m_deferredPassTextures[DeferredPass::Normal].PopTransition(m_pCommandList);
    	m_deferredPassTextures[DeferredPass::Position].PopTransition(m_pCommandList);
    }

    // Render Tonemap Effect
    {
        //NOTE(sean): This is an identical process to the previous block of code, so check that for details

    	ID3D12DescriptorHeap* pHeaps[] = { m_pTonemapResourceDescs->Heap(), };
    	m_pCommandList->SetDescriptorHeaps(_countof(pHeaps), pHeaps);

    	m_tonemapPassTextures[TonemapPass::Color].PushTransition(m_pCommandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        m_pCommandList->OMSetRenderTargets(1, &m_pDeviceResources->GetRenderTargetView(), FALSE, 0);

        m_pTonemapEffect->SetTextures(
            m_pTonemapResourceDescs->GetGpuHandle(TonemapPass::Color)
        );

        m_pTonemapEffect->Apply(m_pCommandList);
        m_pCommandList->DrawInstanced(3, 1, 0, 0);

        m_tonemapPassTextures[TonemapPass::Color].PopTransition(m_pCommandList);
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

void Renderer::LoadAllModels() {
    LoadDebugModel("suzanne", Colors::Peru);
    LoadModel("suzanne", ModelType::Cube);
}

/// Draw an instance of a DebugModel.
/// For example, you might want to render 3 enemies of the same type.
/// Instead of storing an entire copy of the model in each enemy, we store the model once,
/// then reference it with a "handle", in this case a bog-standard index into an array.
/// To move, scasle, and rotate the model, change the world matrix in the instance.
void Renderer::DrawDebugModelInstance(ModelInstance* instance) {
    DebugModel* pModel = &m_debugModels[instance->m_modelIndex];

    switch (pModel->m_modelType) {
    case(DebugModelType::LINE_LIST): {
        BeginDebugLineBatch();
        {
            m_pDebugLineEffect->SetWorld(instance->m_worldMatrix);
            m_pDebugLineEffect->Apply(m_pCommandList);
            DrawDebugLineModel(pModel);
        }
        EndDebugBatch();
    } break;

    case(DebugModelType::TRIANGLE_LIST): {
        BeginDebugTriangleBatch();
        {
            m_pDebugTriangleEffect->SetWorld(instance->m_worldMatrix);
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
void Renderer::LoadModel(const char* name, ModelType model_type) {
    std::string fpath = XMLFindItem(m_pXmlSettings, "models", "model", name);
    ABORT_EQ_FORMAT(fpath, "", "Unable to find \"%s\\%s\\%s\"", "models", "model", name);

    VBOData vbo_data;
    LoadVBO(fpath.c_str(), &vbo_data);

    GameModel* pmodel = &m_models[(u32)model_type];
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

void Renderer::DrawModelInstance(ModelInstance* instance) {
    //TODO(sean): check if this can be moved out when we finalize the debug and game drawing APIs
    m_pDeferredEffect->SetWorld(instance->m_worldMatrix);
    m_pDeferredEffect->SetView(XMLoadFloat4x4(&m_view));

    m_pDeferredEffect->Apply(m_pCommandList);

    GameModel* pmodel = &m_models[instance->m_modelIndex];
    m_pCommandList->IASetVertexBuffers(0, 1, pmodel->vertex_view.get());
    m_pCommandList->IASetIndexBuffer(pmodel->index_view.get());

    m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_pCommandList->DrawIndexedInstanced(pmodel->index_count, 1, 0, 0, 0);
}
