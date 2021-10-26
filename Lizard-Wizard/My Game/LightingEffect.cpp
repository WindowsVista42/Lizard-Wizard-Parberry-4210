#include "LightingEffect.h"
#include <ReadData.h>
#include <iostream>

//NOTE(sean): this is actually a really nice way to do a bitset,
//courtesy of --> https://github.com/microsoft/DirectXTK12/wiki/Authoring-an-Effect
namespace {
    struct __declspec(align(16)) GameEffectConstants {
        u32 light_count;
        u32 _pad0;
        u32 _pad1;
        u32 _pad2;
        Light lights[254];
    };

    static_assert((sizeof(GameEffectConstants) % 16) == 0, "Constant Buffer size alignment");

    constexpr u32 DirtyConstantBuffer = 0x1;
}

//TODO(sean): update this to the right implementation
LightingEffect::LightingEffect(
    ID3D12Device* device,
    const DirectX::EffectPipelineStateDescription& pipeline_state_desc
) :
    m_device(device),
    m_dirtyFlags(u32(-1))
{
    //NOTE(sean): Create root signature for HLSL stuff
    D3D12_ROOT_SIGNATURE_FLAGS root_signature_flags =
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

    CD3DX12_DESCRIPTOR_RANGE texture_range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0);

    CD3DX12_ROOT_PARAMETER root_parameters[PPDescriptors::Count] = {};
    root_parameters[PPDescriptors::InputSRVs].InitAsDescriptorTable(1, &texture_range, D3D12_SHADER_VISIBILITY_PIXEL);
    root_parameters[PPDescriptors::ConstantBuffer].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

    D3D12_STATIC_SAMPLER_DESC static_sampler_desc = {};
    static_sampler_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    static_sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    static_sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    static_sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    static_sampler_desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc = {};
    root_signature_desc.Init(PPDescriptors::Count, root_parameters, 1, &static_sampler_desc, root_signature_flags);

    DX::ThrowIfFailed(
        CreateRootSignature(device, &root_signature_desc, m_rootSignature.ReleaseAndGetAddressOf())
    );

    auto vs_blob = DX::ReadData(L"PostProcessEffect_VS.cso");
    D3D12_SHADER_BYTECODE vs = { vs_blob.data(), vs_blob.size() };

    auto ps_blob = DX::ReadData(L"LightingEffect_PS.cso");
    D3D12_SHADER_BYTECODE ps = { ps_blob.data(), ps_blob.size() };

    pipeline_state_desc.CreatePipelineState(device, m_rootSignature.Get(), vs, ps, m_pso.ReleaseAndGetAddressOf());
}

void LightingEffect::SetTextures(DescriptorHeap* textures) {
    m_colorTexture = textures->GetFirstGpuHandle();
}

void LightingEffect::SetLightCount(u32 light_count) {
    this->light_count = light_count;
    m_dirtyFlags |= DirtyConstantBuffer;
}

void LightingEffect::SetLight(usize index, Light light) {
    lights[index] = light;
}

Light* LightingEffect::Lights() {
    return lights;
}

//TODO(sean): update this to the right implementation
void LightingEffect::Apply(ID3D12GraphicsCommandList* command_list) {
    //NOTE(sean): update dirty data
    if (m_dirtyFlags & DirtyConstantBuffer) {
        auto constant_buffer = GraphicsMemory::Get(m_device.Get()).AllocateConstant<GameEffectConstants>();

        GameEffectConstants data = {
            light_count
        };

        memcpy((u8*)constant_buffer.Memory(), &data, sizeof(u32));
        memcpy((u8*)constant_buffer.Memory() + offsetof(GameEffectConstants, lights), lights, sizeof(Light) * _countof(lights));
        std::swap(m_constantBuffer, constant_buffer);

        m_dirtyFlags &= ~DirtyConstantBuffer;
    }

    //NOTE(sean): set root signature and parameters
    command_list->SetGraphicsRootSignature(m_rootSignature.Get());

    //NOTE(sean): set render resources
    command_list->SetGraphicsRootDescriptorTable(0, m_colorTexture);

    //NOTE(sean): set render constants
    command_list->SetGraphicsRootConstantBufferView(1, m_constantBuffer.GpuAddress());

    //NOTE(sean): set pipeline state
    command_list->SetPipelineState(m_pso.Get());
}
