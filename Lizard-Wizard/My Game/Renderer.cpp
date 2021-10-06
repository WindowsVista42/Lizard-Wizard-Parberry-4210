#include "Renderer.h"
#include "Math.h"
#include <Model.h>
#include <iostream>

const u32 g_cubeVertexCount = 8;
const Vector3 g_cubeVertices[g_cubeVertexCount] = {
    Vector3(1.0, 1.0, 1.0),
    Vector3(1.0, 1.0, 0.0),
    Vector3(1.0, 0.0, 1.0),
    Vector3(1.0, 0.0, 0.0),
    Vector3(0.0, 1.0, 1.0),
    Vector3(0.0, 1.0, 0.0),
    Vector3(0.0, 0.0, 1.0),
    Vector3(0.0, 0.0, 0.0),
};

//TODO(sean): get proper cube indices
const u32 g_cubeIndexCount = 6;
const u32 g_cubeIndices[g_cubeIndexCount] = {
    0, 1, 2,
    3, 2, 1
};

CRenderer::CRenderer():
    LRenderer3D(),
    m_pCamera(new LBaseCamera),
    m_debugScratch(16 * 1024) // 16k
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

    //m_pCamera->SetOrthographic(width, height, near_clip, far_clip);
}

CRenderer::~CRenderer() {
    delete m_pCamera;
    m_pDeviceResources->WaitForGpu();
}

void CRenderer::Initialize() {
    LRenderer3D::Initialize();

    CreateAllEffects();
    CreateCubeBuffers();
}

void CRenderer::CreateAllEffects() {
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

    /*
    //TODO(sean): Move this into member variables
    m_pLineEffect->EnableDefaultLighting();
    m_pLineEffect->SetFogColor(Colors::Black);
    m_pLineEffect->SetFogStart(10.0);
    m_pLineEffect->SetFogEnd(100.0);
    m_pLineEffect->SetDiffuseColor(Colors::Red);
    */
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

void CRenderer::CreateCubeBuffers() {
    { //NOTE(sean): vertex buffer
        VertexPC vertices[g_cubeVertexCount];
        //Vector3 normal(-Vector3::UnitZ);
        //Vector2 texture = Vector2(0.0, 0.0);
        Vector3 color = Colors::Red;

        for (Vector3 position: g_cubeVertices) {
            vertices[0] = VertexPC(position, Colors::Red);
        }

        auto data = (u8*)&vertices;
        const isize size = g_cubeVertexCount * sizeof(VertexPC);
        CreateBufferAndView<VertexPC>(data, size, m_cubeVertexBuffer, m_pVertexBufferView);
    }

    { //NOTE(sean): index buffer
        auto data = (u8*)&g_cubeIndices;
        const isize size = g_cubeIndexCount * sizeof(u32);
        CreateBufferAndView(data, size, m_cubeIndexBuffer, m_pIndexBufferView);
    }
}

/// <summary>
/// Begin Rendering a frame.
/// Put all DrawXYZ() or other functions in between this and EndFrame()
/// </summary>
void CRenderer::BeginFrame() {
    LRenderer3D::BeginFrame();
}

/// <summary>
/// End Rendering a frame.
/// Put all DrawXYZ() or other functions in between this and BeginFrame()
/// </summary>
void CRenderer::EndFrame() {
    LRenderer3D::EndFrame();
    m_frameNumber += 1;
}

/*
// TODO(sean): Implement the actual renderer
/// <summary>
/// Render a cube at the position.
/// </summary>
/// <param name="cube_position"></param>
void CRenderer::DrawCube(const Vector3& position) {
    const XMVECTORF32 scale = { 1.0f, 1.0f, 1.0f };
    const XMVECTORF32 translate = { position.x, position.y, position.z };

    XMMATRIX world = XMMatrixTransformation( g_XMZero, Quaternion::Identity, scale, g_XMZero, Quaternion::Identity, translate);

    m_pDebugLineEffect->SetWorld(world);
    m_pDebugLineEffect->SetView(XMLoadFloat4x4(&m_view));

    m_pDebugLineEffect->Apply(m_pCommandList);

    m_pCommandList->IASetVertexBuffers(0, 1, m_pVertexBufferView.get());
    m_pCommandList->IASetIndexBuffer(m_pIndexBufferView.get());
    m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //NOTE(sean): we want a triangle list here for custom meshes
    m_pCommandList->DrawIndexedInstanced(g_cubeIndexCount, 1, 0, 0, 0);
}
*/

void CRenderer::BeginDebugLineBatch() {
    m_pPrimitiveBatch->Begin(m_pCommandList);

    XMMATRIX world = XMMatrixTransformation( g_XMZero, Quaternion::Identity, g_XMOne, g_XMZero, Quaternion::Identity, g_XMZero);

    m_pDebugLineEffect->SetWorld(world);
    m_pDebugLineEffect->SetView(XMLoadFloat4x4(&m_view));
    m_pDebugLineEffect->Apply(m_pCommandList);
}

void CRenderer::BeginDebugTriangleBatch() {
    m_pPrimitiveBatch->Begin(m_pCommandList);

    m_pDebugTriangleEffect->SetView(XMLoadFloat4x4(&m_view));
    m_pDebugTriangleEffect->Apply(m_pCommandList);
}

void CRenderer::EndDebugBatch() {
    m_pPrimitiveBatch->End();
}

void CRenderer::DrawDebugLine(
    const Vector3 A,
    const Vector3 B,
    const XMVECTORF32 color
) {
    m_pPrimitiveBatch->DrawLine(VertexPC(A, color), VertexPC(B, color));
}

void CRenderer::DrawDebugTriangle(
    const Vector3 A,
    const Vector3 B,
    const Vector3 C,
    const XMVECTORF32 color
) {
    VertexPC vertices[4] = { VertexPC(A, color), VertexPC(B, color), VertexPC(C, color), VertexPC(A, color) };
    m_pPrimitiveBatch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, vertices, 4);
}

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

void CRenderer::DrawDebugRay(
    const Vector3 origin,
    const Vector3 direction,
    const f32 length,
    const XMVECTORF32 color
) {
    const Vector3 end = origin + direction * length;
    DrawDebugLine(origin, end, color);
}

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
void CRenderer::DrawDebugAABB(
    const BoundingBox box,
    const XMVECTORF32 color
) {
    XMMATRIX world = MoveScaleMatrix(box.Center, box.Extents);
    DrawDebugCubeInternal(world, color);
}

//NOTE(sean): https://github.com/Microsoft/DirectXTK/wiki/DebugDraw
void CRenderer::DrawDebugOBB(
    const BoundingOrientedBox obb,
    const XMVECTORF32 color
) {
    XMMATRIX world = MoveRotateScaleMatrix(obb.Center, obb.Orientation, obb.Extents);
    DrawDebugCubeInternal(world, color);
}

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

u32 CRenderer::AddDebugModel(SDebugModel* model) {
    m_debugModels.push_back(*model);
    return m_debugModels.size() - 1;
}

void CRenderer::LoadAllModels() {
    LoadDebugModel("suzanne", Colors::Peru);
}

/*
void CRenderer::DrawDebugModelInstance(SModelInstance* instance) {
    m_pDebugEffect->SetWorld(instance->m_worldMatrix);
    m_pDebugEffect->Apply(m_pCommandList);

    DrawDebugModel(&m_debugModels[instance->m_modelIndex]);
}

*/

void CRenderer::DrawDebugModelInstance(SModelInstance* instance) {
    SDebugModel* pModel = &m_debugModels[instance->m_modelIndex];

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

void CRenderer::ResetDebugWorldMatrix() {
    //NOTE(sean): we might want to look into not making this happen on every draw call
    const XMMATRIX world = XMMatrixTransformation(g_XMZero, Quaternion::Identity, g_XMOne, g_XMZero, Quaternion::Identity, g_XMZero);
    m_pDebugLineEffect->SetWorld(world);
    m_pDebugLineEffect->Apply(m_pCommandList);
}

//NOTE(sean): if performance is required, this can be made nocopy
void CRenderer::DrawDebugLineModel(SDebugModel* model) {
    m_pPrimitiveBatch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINELIST, model->m_vertexList.data(), model->m_vertexList.size());
}

void CRenderer::DrawDebugTriangleModel(SDebugModel* model) {
    m_pPrimitiveBatch->Draw(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, model->m_vertexList.data(), model->m_vertexList.size());
}

const usize CRenderer::GetNumFrames() const {
    return m_frameNumber;
}

HWND CRenderer::GetHwnd() {
    return m_Hwnd;
}

u32 CRenderer::GetResolutionWidth() {
    return m_nWinWidth;
}

u32 CRenderer::GetResolutionHeight() {
    return m_nWinHeight;
}

u32 CRenderer::LoadDebugModel(const char* name, XMVECTORF32 color) {
    // LSpriteRenderer::LoadByIndex() used as ref

    if (m_pXmlSettings == 0) { ABORT("Unable to find gamesettings.xml");  }

    XMLElement* models_tag = m_pXmlSettings->FirstChildElement("models");
    if (models_tag == 0) { ABORT("Unable to find \"models\" tag"); }

    std::string path(models_tag->Attribute("path"));
    XMLElement* model_tag = models_tag->FirstChildElement("model");

    // find specific tag
    while (model_tag != 0 && strcmp(name, model_tag->Attribute("name"))) {
        model_tag = model_tag->NextSiblingElement("model");
    }

    // was not found
    if (model_tag == 0) { ABORT("Unable to find name tag");  }

    if (model_tag->Attribute("file")) {
        const std::string fpath = path + "\\" + model_tag->Attribute("file");

        //NOTE(sean): VBO file format: https://github.com/microsoft/DirectXMesh/blob/master/Meshconvert/Mesh.cpp
        struct VBOVertex {
            XMFLOAT3 position;
            XMFLOAT3 normal;
            XMFLOAT2 texture;
        };

        //TODO(sean): load models from vbo file
        u32 vertex_count = 0;
        u32 index_count = 0;
        VBOVertex* vertices = 0;
        u16* indices = 0;

        {
            FILE* fp = fopen(fpath.c_str(), "rb");
            if (fp == 0) { ABORT("Attempted to open VBO file and failed!"); }

            fread(&vertex_count, sizeof(u32), 1, fp);
            if (vertex_count == 0) { ABORT("Input VBO has ZERO vertices!"); }

            fread(&index_count, sizeof(u32), 1, fp);
            if (index_count == 0) { ABORT("Input VBO has ZERO indices!"); }

            vertices = new VBOVertex[vertex_count];
            if (vertices == 0) { ABORT("Failed to allocate VBO vertex buffer!"); }

            indices = new u16[index_count];
            if (indices == 0) { ABORT("Failed to allocate VBO index buffer!"); }

            //TODO(sean): error checking?
            fread(vertices, sizeof(VBOVertex), vertex_count, fp);
            fread(indices, sizeof(u16), index_count, fp);

            fclose(fp);
        }

        //NOTE(sean): closest number under 2048 that is a multiple of 3,
        //this seems to be the limit for the current primitive batch
        if (index_count > 2046) {
            ABORT("Input VBO must not contain more than 2046 indices!");
        }

        VertexPC* mesh = new VertexPC[index_count];

        // unpack index list
        for every(index, index_count) {
            mesh[index] = { vertices[indices[index]].position, *(XMFLOAT4*)&color }; // position and color
        }

        delete[] vertices;
        delete[] indices;

        u32 handle = AddDebugModel(&SDebugModel(mesh, index_count, DebugModelType::LINE_LIST));

        delete[] mesh;

        return handle;
    } else {
        ABORT("Unable to find file."); // ABORT
    }
}
