#include "EffectsCommon.h"
#include <ReadData.h>

void CreatePostProcessPassData(
    ID3D12Device* device,
    const DirectX::EffectPipelineStateDescription& pipeline_state_desc,
    Microsoft::WRL::ComPtr<ID3D12RootSignature>& m_rootSignature,
    Microsoft::WRL::ComPtr<ID3D12PipelineState>& m_pso,
    const u32 input_texture_count,
    const wchar* pixel_shader
) {
    //NOTE(sean): Create root signature for HLSL stuff
    D3D12_ROOT_SIGNATURE_FLAGS root_signature_flags =
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

    CD3DX12_DESCRIPTOR_RANGE texture_range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, input_texture_count, 0);

    CD3DX12_ROOT_PARAMETER root_parameters[2] = {};
    root_parameters[0].InitAsDescriptorTable(1, &texture_range, D3D12_SHADER_VISIBILITY_PIXEL);
    root_parameters[1].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

    D3D12_STATIC_SAMPLER_DESC static_sampler_desc = {};
    static_sampler_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    static_sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    static_sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    static_sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    static_sampler_desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc = {};
    root_signature_desc.Init(2, root_parameters, 1, &static_sampler_desc, root_signature_flags);

    DX::ThrowIfFailed(
        CreateRootSignature(device, &root_signature_desc, m_rootSignature.ReleaseAndGetAddressOf())
    );

    auto vs_blob = DX::ReadData(L"PostProcess_VS.cso");
    D3D12_SHADER_BYTECODE vs = { vs_blob.data(), vs_blob.size() };

    auto ps_blob = DX::ReadData(pixel_shader);
    D3D12_SHADER_BYTECODE ps = { ps_blob.data(), ps_blob.size() };

    pipeline_state_desc.CreatePipelineState(device, m_rootSignature.Get(), vs, ps, m_pso.ReleaseAndGetAddressOf());
}

