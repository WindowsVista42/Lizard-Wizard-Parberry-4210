#include "DeferredEffect.h"
#include <ReadData.h>

//NOTE(sean): this is actually a really nice way to do a bitset,
//courtesy of --> https://github.com/microsoft/DirectXTK12/wiki/Authoring-an-Effect
namespace {
    struct __declspec(align(16)) GameEffectConstants {
        XMMATRIX worldViewProjection;
        XMMATRIX world;
        Vec4 glow;
    };

    static_assert((sizeof(GameEffectConstants) % 16) == 0, "Constant Buffer size alignment");

    constexpr u32 DirtyConstantBuffer = 0x1;
    constexpr u32 DirtyWorldViewProjectionMatrix = 0x2;
    constexpr u32 DirtyWorldMatrix = 0x4;
    //NOTE(sean): other bit flags go here
}

DeferredEffect::DeferredEffect(
    ID3D12Device* device,
    const DirectX::EffectPipelineStateDescription& pipeline_state_desc
):
    m_device(device),
    m_dirtyFlags(u32(-1)),
    m_world(XMMatrixIdentity()),
    m_view(XMMatrixIdentity()),
    m_projection(XMMatrixIdentity())
{

    //NOTE(sean): Create root signature for HLSL stuff
    D3D12_ROOT_SIGNATURE_FLAGS root_signature_flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

    CD3DX12_DESCRIPTOR_RANGE texture_range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

    CD3DX12_ROOT_PARAMETER root_parameters[Descriptors::Count] = {};
    root_parameters[Descriptors::InputSRV].InitAsDescriptorTable(1, &texture_range, D3D12_SHADER_VISIBILITY_PIXEL);
    root_parameters[Descriptors::ConstantBuffer].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);

    D3D12_STATIC_SAMPLER_DESC static_sampler_desc = {};
    static_sampler_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    static_sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    static_sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    static_sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    static_sampler_desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc = {};
    root_signature_desc.Init(Descriptors::Count, root_parameters, 1, &static_sampler_desc, root_signature_flags);

    DX::ThrowIfFailed(
        CreateRootSignature(device, &root_signature_desc, m_rootSignature.ReleaseAndGetAddressOf())
    );

    auto vs_blob = DX::ReadData(L"Deferred_VS.cso");
    D3D12_SHADER_BYTECODE vs = { vs_blob.data(), vs_blob.size() };

    auto ps_blob = DX::ReadData(L"Deferred_PS.cso");
    D3D12_SHADER_BYTECODE ps = { ps_blob.data(), ps_blob.size() };

    pipeline_state_desc.CreatePipelineState(device, m_rootSignature.Get(), vs, ps, m_pso.ReleaseAndGetAddressOf());
}

void DeferredEffect::SetTextures(D3D12_GPU_DESCRIPTOR_HANDLE first_texture) {
    m_firstTexture = first_texture;
}


void DeferredEffect::Apply(ID3D12GraphicsCommandList* command_list) {
    //NOTE(sean): update dirty data
    if (m_dirtyFlags & DirtyWorldViewProjectionMatrix) {
        Mat4x4 world_view = XMMatrixMultiply(m_world, m_view);
        m_worldViewProjection = XMMatrixTranspose(XMMatrixMultiply(world_view, m_projection));

        m_dirtyFlags &= ~DirtyWorldViewProjectionMatrix;
        m_dirtyFlags |= DirtyConstantBuffer;
    }

    if (m_dirtyFlags & DirtyWorldMatrix) {
        Mat4x4 world = XMMatrixTranspose(m_world);
        m_world = world;

        m_dirtyFlags &= ~DirtyWorldMatrix;
        m_dirtyFlags |= DirtyConstantBuffer;
    }

    if (m_dirtyFlags & DirtyConstantBuffer) {
        auto constant_buffer = GraphicsMemory::Get(m_device.Get()).AllocateConstant<GameEffectConstants>();

        GameEffectConstants data = {};
        data.worldViewProjection = m_worldViewProjection;
        data.world = m_world;
        data.glow = m_glow;

        memcpy(constant_buffer.Memory(), &data, constant_buffer.Size());
        std::swap(m_constantBuffer, constant_buffer);

        m_dirtyFlags &= ~DirtyConstantBuffer;
    }

    //NOTE(sean): set root signature and parameters
    command_list->SetGraphicsRootSignature(m_rootSignature.Get());

    //NOTE(sean): set textures
    command_list->SetGraphicsRootDescriptorTable(0, m_firstTexture);

    //NOTE(sean): set render constants
    command_list->SetGraphicsRootConstantBufferView(Descriptors::ConstantBuffer, m_constantBuffer.GpuAddress());

    //NOTE(sean): set pipeline state
    command_list->SetPipelineState(m_pso.Get());
}

void XM_CALLCONV DeferredEffect::SetWorld(DirectX::FXMMATRIX world) {
    m_world = world;
    m_dirtyFlags |= DirtyWorldViewProjectionMatrix | DirtyWorldMatrix;
}

void XM_CALLCONV DeferredEffect::SetView(DirectX::FXMMATRIX view) {
    m_view = view;
    m_dirtyFlags |= DirtyWorldViewProjectionMatrix;
}

void XM_CALLCONV DeferredEffect::SetProjection(DirectX::FXMMATRIX projection) {
    m_projection = projection;
    m_dirtyFlags |= DirtyWorldViewProjectionMatrix;
}

void XM_CALLCONV DeferredEffect::SetMatrices(DirectX::FXMMATRIX world, DirectX::CXMMATRIX view, DirectX::CXMMATRIX projection) {
    m_world = world;
    m_view = view;
    m_projection = projection;
    m_dirtyFlags |= DirtyWorldViewProjectionMatrix;
}

void DeferredEffect::SetGlow(Vec3 glow) {
    m_glow = Vec4(glow.x, glow.y, glow.z, 0.0f) + Vec4(1.0f);
    m_dirtyFlags |= DirtyConstantBuffer;
}
