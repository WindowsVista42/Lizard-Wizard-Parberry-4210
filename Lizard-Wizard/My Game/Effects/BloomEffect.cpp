#include "BloomEffect.h"

// SHARED

namespace {
    constexpr u32 DirtyConstantBuffer = 0x1;
}

// BLOOM EXTRACT

namespace {
    struct __declspec(align(16)) BloomExtractConstants {
    };

    static_assert((sizeof(BloomExtractConstants) % 16) == 0, "Constant Buffer size alignment");
}

BloomExtractEffect::BloomExtractEffect(
    ID3D12Device* device,
    const DirectX::EffectPipelineStateDescription& pipeline_state_desc
) :
    m_device(device),
    m_dirtyFlags(u32(-1))
{
    CreatePostProcessPassData(device, pipeline_state_desc, m_rootSignature, m_pso, 1, L"BloomExtract_PS.cso");
}

void BloomExtractEffect::SetTextures(DescriptorHeap* textures) {
    m_firstTexture = textures->GetFirstGpuHandle();
}

void BloomExtractEffect::Apply(ID3D12GraphicsCommandList* command_list) {
}

// BLOOM BLUR

namespace {
    struct __declspec(align(16)) BloomBlurConstants {
    };

    static_assert((sizeof(BloomBlurConstants) % 16) == 0, "Constant Buffer size alignment");
}

BloomBlurEffect::BloomBlurEffect(
    ID3D12Device* device,
    const DirectX::EffectPipelineStateDescription& pipeline_state_desc
) :
    m_device(device),
    m_dirtyFlags(u32(-1))
{
    CreatePostProcessPassData(device, pipeline_state_desc, m_rootSignature, m_pso, 1, L"BloomBlur_PS.cso");
}

void BloomBlurEffect::SetTextures(DescriptorHeap* textures) {
    m_firstTexture = textures->GetFirstGpuHandle();
}

void BloomBlurEffect::Apply(ID3D12GraphicsCommandList* command_list) {
}

// BLOOM COMBINE

namespace {
    struct __declspec(align(16)) BloomCombineConstants {
    };

    static_assert((sizeof(BloomCombineConstants) % 16) == 0, "Constant Buffer size alignment");
}

BloomCombineEffect::BloomCombineEffect(
    ID3D12Device* device,
    const DirectX::EffectPipelineStateDescription& pipeline_state_desc
) :
    m_device(device),
    m_dirtyFlags(u32(-1))
{
    CreatePostProcessPassData(device, pipeline_state_desc, m_rootSignature, m_pso, 1, L"BloomCombine_PS.cso");
}

void BloomCombineEffect::SetTextures(DescriptorHeap* textures) {
    m_firstTexture = textures->GetFirstGpuHandle();
}

void BloomCombineEffect::Apply(ID3D12GraphicsCommandList* command_list) {
}
