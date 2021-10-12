#include "Renderer.h"
#include "Math.h"
#include "Helpers.h"
#include <Model.h>
#include <iostream>

CRenderer::CRenderer():
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

CRenderer::~CRenderer() {
    delete m_pCamera;
    m_pDeviceResources->WaitForGpu();
}

void CRenderer::Initialize() {
    LRenderer3D::Initialize();

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

        //TODO(sean): Figure out what the extra fields for this *can* do
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

        //TODO(sean): Figure out what the extra fields for this *can* do
        m_pDebugTriangleEffect = std::make_unique<BasicEffect>(m_pD3DDevice, EffectFlags::VertexColor, pipeline_state_desc);
        m_pDebugTriangleEffect->SetProjection(XMLoadFloat4x4(&m_projection));
    }

    {
        EffectPipelineStateDescription pipeline_state_desc(
            &VertexPNT::InputLayout,
            CommonStates::NonPremultiplied,
            CommonStates::DepthDefault,
            CommonStates::CullNone,
            m_RenderTargetState
        );

        m_pGameEffect = std::make_unique<GameEffect>(m_pD3DDevice, pipeline_state_desc);
        m_pGameEffect->SetProjection(XMLoadFloat4x4(&m_projection));
    }
}

/// Begin Rendering a frame.
/// Put all DrawXYZ() or other functions in between this and EndFrame()
void CRenderer::BeginFrame() {
    LRenderer3D::BeginFrame();
}

/// End Rendering a frame.
/// Put all DrawXYZ() or other functions in between this and BeginFrame()
void CRenderer::EndFrame() {
    LRenderer3D::EndFrame();
    m_frameNumber += 1;
}

/// Begins a debug line batch.
/// Code that follows this will be batched into ONE draw call
/// Needs to use compatable functions
void CRenderer::BeginDebugLineBatch() {
    m_pPrimitiveBatch->Begin(m_pCommandList);

    XMMATRIX world = XMMatrixTransformation( g_XMZero, Quaternion::Identity, g_XMOne, g_XMZero, Quaternion::Identity, g_XMZero);

    m_pDebugLineEffect->SetWorld(world);
    m_pDebugLineEffect->SetView(XMLoadFloat4x4(&m_view));
    m_pDebugLineEffect->Apply(m_pCommandList);
}

/// Begins a debug triangle batch.
/// Code that follows this will be batched into ONE draw call
/// Needs to use compatable functions
void CRenderer::BeginDebugTriangleBatch() {
    m_pPrimitiveBatch->Begin(m_pCommandList);

    m_pDebugTriangleEffect->SetView(XMLoadFloat4x4(&m_view));
    m_pDebugTriangleEffect->Apply(m_pCommandList);
}

/// Ends any currently open batch
/// Use this to "draw" the current batched items, however actual rendering is technically performed later
void CRenderer::EndDebugBatch() {
    m_pPrimitiveBatch->End();
}

/// Draw a colored "Debug Line" from A to B
/// A - B
void CRenderer::DrawDebugLine(
    const Vector3 A,
    const Vector3 B,
    const XMVECTORF32 color
) {
    m_pPrimitiveBatch->DrawLine(VertexPC(A, color), VertexPC(B, color));
}

/// Draw a colored "Debug Triangle".
///   A
///  / \
/// C - B
void CRenderer::DrawDebugTriangle(
    const Vector3 A,
    const Vector3 B,
    const Vector3 C,
    const XMVECTORF32 color
) {
    VertexPC vertices[4] = { VertexPC(A, color), VertexPC(B, color), VertexPC(C, color), VertexPC(A, color) };
    m_pPrimitiveBatch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, vertices, 4);
}

/// Draw a colored "Debug Quad".
/// A - B
/// |   |
/// D - C
void CRenderer::DrawDebugQuad(
    const Vector3 A,
    const Vector3 B,
    const Vector3 C,
    const Vector3 D,
    const XMVECTORF32 color
) {
    VertexPC vertices[5] = { VertexPC(A, color), VertexPC(B, color), VertexPC(C, color), VertexPC(D, color), VertexPC(A, color) };
    m_pPrimitiveBatch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, vertices, 5);
}

/// Draw a colored "Debug Ray".
/// A --->

void CRenderer::DrawDebugRay(
    const Vector3 origin,
    const Vector3 direction,
    const f32 length,
    const XMVECTORF32 color
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
void CRenderer::DrawDebugRing(
    const Vector3 origin,
    const Vector3 orientation,
    const f32 radius,
    const u32 segments,
    const XMVECTORF32 color
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
void CRenderer::DrawDebugRing2(
    FXMVECTOR origin,
    FXMVECTOR majorAxis,
    FXMVECTOR minorAxis,
    const u32 segments,
    const XMVECTORF32 color
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
void CRenderer::DrawDebugSphere(
    const BoundingSphere sphere,
    const u32 segments,
    const XMVECTORF32 color
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
void CRenderer::DrawDebugAABB(
    const BoundingBox box,
    const XMVECTORF32 color
) {
    XMMATRIX world = MoveScaleMatrix(box.Center, box.Extents);
    DrawDebugCubeInternal(world, color);
}

//NOTE(sean): https://github.com/Microsoft/DirectXTK/wiki/DebugDraw
/// Draw a colored box, but it's rotated
void CRenderer::DrawDebugOBB(
    const BoundingOrientedBox obb,
    const XMVECTORF32 color
) {
    XMMATRIX world = MoveRotateScaleMatrix(obb.Center, obb.Orientation, obb.Extents);
    DrawDebugCubeInternal(world, color);
}

/// Draw a colored "Debug Capsule"
void CRenderer::DrawDebugCapsule(
   const Vector3 origin, 
   const f32 radius, 
   const f32 height, 
   const u32 segments,
   const XMVECTORF32 color
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
void CRenderer::DrawDebugGrid(
    const Vector3 x_axis,
    const Vector3 y_axis,
    const Vector3 origin,
    const u32 x_segments,
    const u32 y_segments,
    const XMVECTORF32 color
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
void CRenderer::DrawDebugCubeInternal(
    const CXMMATRIX world,
    const XMVECTORF32 color
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
u32 CRenderer::AddDebugModel(DebugModel* model) {
    m_debugModels.push_back(*model);
    return m_debugModels.size() - 1;
}

void CRenderer::LoadAllModels() {
    LoadDebugModel("untitled", Colors::Peru);
    LoadModel("untitled", ModelType::Cube);
}

/*
void CRenderer::DrawDebugModelInstance(SModelInstance* instance) {
    m_pDebugEffect->SetWorld(instance->m_worldMatrix);
    m_pDebugEffect->Apply(m_pCommandList);

    DrawDebugModel(&m_debugModels[instance->m_modelIndex]);
}

*/

/// Draw an instance of a DebugModel.
/// For example, you might want to render 3 enemies of the same type.
/// Instead of storing an entire copy of the model in each enemy, we store the model once,
/// then reference it with a "handle", in this case a bog-standard index into an array.
/// To move, scasle, and rotate the model, change the world matrix in the instance.
void CRenderer::DrawDebugModelInstance(ModelInstance* instance) {
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
void CRenderer::ResetDebugWorldMatrix() {
    //NOTE(sean): we might want to look into not making this happen on every draw call
    const XMMATRIX world = XMMatrixIdentity();
    m_pDebugLineEffect->SetWorld(world);
    m_pDebugLineEffect->Apply(m_pCommandList);
}

//NOTE(sean): if performance is required, this can be made nocopy
/// Internal method to draw a DebugModel
void CRenderer::DrawDebugLineModel(DebugModel* model) {
    m_pPrimitiveBatch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINELIST, model->m_vertexList.data(), model->m_vertexList.size());
}

/// Internal method to draw a DebugModel
void CRenderer::DrawDebugTriangleModel(DebugModel* model) {
    m_pPrimitiveBatch->Draw(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, model->m_vertexList.data(), model->m_vertexList.size());
}

/// Get the number of frames elapsed since the start of the program
const usize CRenderer::GetNumFrames() const {
    return m_frameNumber;
}

/// Get the windows Hwnd (Window Handle)
HWND CRenderer::GetHwnd() {
    return m_Hwnd;
}

/// Get the horizontal resolution
u32 CRenderer::GetResolutionWidth() {
    return m_nWinWidth;
}

/// Get the vertical resolution
u32 CRenderer::GetResolutionHeight() {
    return m_nWinHeight;
}

//NOTE(sean): VBO file format: https://github.com/microsoft/DirectXMesh/blob/master/Meshconvert/Mesh.cpp
struct VBOData {
    u32 vertex_count;
    u32 index_count;
    VertexPNT* vertices;
    u16* indices;

    void free() {
        delete[] vertices;
        delete[] indices;
    }
};

//NOTE(sean): VBO file format: https://github.com/microsoft/DirectXMesh/blob/master/Meshconvert/Mesh.cpp
void LoadVBO(const char* fpath, VBOData* model) {
    // TODO(sean): error checking?

    FILE* fp = fopen(fpath, "rb");

    fread(&model->vertex_count, sizeof(u32), 1, fp);
    fread(&model->index_count, sizeof(u32), 1, fp);

    model->vertices = new VertexPNT[model->vertex_count];
    model->indices = new u16[model->index_count];

    fread(model->vertices, sizeof(VertexPNT), model->vertex_count, fp);
    fread(model->indices, sizeof(u16), model->index_count, fp);

    fclose(fp);
}

/// Load a debug model with the name.
/// Debug models are assumed to be VBO (.vbo) files.
u32 CRenderer::LoadDebugModel(const char* name, XMVECTORF32 color) {
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

template <class T>
void CreateBufferAndView(u8* data, isize size, GraphicsResource& resource, std::shared_ptr<D3D12_VERTEX_BUFFER_VIEW>& view) {
        resource = GraphicsMemory::Get().Allocate(size);
        memcpy(resource.Memory(), data, size); // i like this function

        view = std::make_shared<D3D12_VERTEX_BUFFER_VIEW>();
        view->BufferLocation = resource.GpuAddress();
        view->StrideInBytes = sizeof(T);
        view->SizeInBytes = (u32)resource.Size();
}

void CreateBufferAndView(u8* data, isize size, GraphicsResource& resource, std::shared_ptr<D3D12_INDEX_BUFFER_VIEW>& view) {
        resource = GraphicsMemory::Get().Allocate(size);
        memcpy(resource.Memory(), data, size);

        view = std::make_shared<D3D12_INDEX_BUFFER_VIEW>();
        view->BufferLocation = resource.GpuAddress();
        view->SizeInBytes = (u32)resource.Size();
        view->Format = DXGI_FORMAT_R32_UINT;
}

/// Load a model.
/// Models are assumed to be VBO (.vbo) files.
void CRenderer::LoadModel(const char* name, ModelType model_type) {
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

    { //NOTE(sean): vertex buffer
        auto data = (u8*)vbo_data.vertices;
        const isize size = vbo_data.vertex_count * sizeof(VertexPNT);
        CreateBufferAndView<VertexPNT>(data, size, pmodel->vertex_buffer, pmodel->vertex_view);
    }

    { //NOTE(sean): index buffer
        auto data = (u8*)indices;
        const isize size = vbo_data.index_count * sizeof(u32);
        CreateBufferAndView(data, size, pmodel->index_buffer, pmodel->index_view);
    }

    vbo_data.free();
    delete[] indices;
}

void CRenderer::RenderInstance(ModelInstance* instance) {
    m_pGameEffect->SetWorld(instance->m_worldMatrix);
    m_pGameEffect->SetView(XMLoadFloat4x4(&m_view));

    m_pGameEffect->Apply(m_pCommandList);

    GameModel* pmodel = &m_models[instance->m_modelIndex];
    m_pCommandList->IASetVertexBuffers(0, 1, pmodel->vertex_view.get());
    m_pCommandList->IASetIndexBuffer(pmodel->index_view.get());

    m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_pCommandList->DrawIndexedInstanced(pmodel->index_count, 1, 0, 0, 0);
}
