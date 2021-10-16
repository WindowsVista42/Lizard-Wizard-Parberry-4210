#include "PostProcessEffect.h"
#include <ReadData.h>

//NOTE(sean): this is actually a really nice way to do a bitset,
//courtesy of --> https://github.com/microsoft/DirectXTK12/wiki/Authoring-an-Effect
namespace {
    struct __declspec(align(16)) GameEffectConstants {};

    static_assert((sizeof(GameEffectConstants) % 16) == 0, "Constant Buffer size alignment");

    constexpr u32 DirtyConstantBuffer = 0x1;
    //NOTE(sean): other bit flags go here
}

//TODO(sean): update this to the right implementation
PostProcessEffect::PostProcessEffect(
    ID3D12Device* device,
    const DirectX::EffectPipelineStateDescription& pipeline_state_desc
) :
    m_device(device),
    m_dirtyFlags(u32(-1))
{
    //NOTE(sean): Create root signature for HLSL stuff
    D3D12_ROOT_SIGNATURE_FLAGS root_signature_flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

    CD3DX12_ROOT_PARAMETER root_parameters[Descriptors::Count] = {};
    root_parameters[Descriptors::ConstantBuffer].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc = {};
    root_signature_desc.Init(Descriptors::Count, root_parameters, 0, 0, root_signature_flags);

    DX::ThrowIfFailed(
        CreateRootSignature(device, &root_signature_desc, m_rootSignature.ReleaseAndGetAddressOf())
    );

    auto vs_blob = DX::ReadData(L"PostProcessEffect_VS.cso");
    D3D12_SHADER_BYTECODE vs = { vs_blob.data(), vs_blob.size() };

    auto ps_blob = DX::ReadData(L"PostProcessEffect_PS.cso");
    D3D12_SHADER_BYTECODE ps = { ps_blob.data(), ps_blob.size() };

    pipeline_state_desc.CreatePipelineState(device, m_rootSignature.Get(), vs, ps, m_pso.ReleaseAndGetAddressOf());
}

//TODO(sean): update this to the right implementation
void PostProcessEffect::Apply(ID3D12GraphicsCommandList* command_list) {
    //NOTE(sean): update dirty data
    if (m_dirtyFlags & DirtyConstantBuffer) {
        auto constant_buffer = GraphicsMemory::Get(m_device.Get()).AllocateConstant<GameEffectConstants>();

        GameEffectConstants data = {};

        memcpy(constant_buffer.Memory(), &data, constant_buffer.Size());
        std::swap(m_constantBuffer, constant_buffer);

        m_dirtyFlags &= ~DirtyConstantBuffer;
    }

    //NOTE(sean): set root signature and parameters
    command_list->SetGraphicsRootSignature(m_rootSignature.Get());

    //NOTE(sean): validate some members here

    //NOTE(sean): set render constants
    command_list->SetGraphicsRootConstantBufferView(Descriptors::ConstantBuffer, m_constantBuffer.GpuAddress());

    //NOTE(sean): set pipeline state
    command_list->SetPipelineState(m_pso.Get());
}
