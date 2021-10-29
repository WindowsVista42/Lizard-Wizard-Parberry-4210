#include "BloomEffect.h"

// SHARED

namespace {
    constexpr u32 DirtyConstantBuffer = 0x1;
}

// BLOOM EXTRACT

namespace {
    struct __declspec(align(16)) BloomExtractConstants {
        f32 threshold;
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

void BloomExtractEffect::SetTextures(D3D12_GPU_DESCRIPTOR_HANDLE first_texture) {
    m_firstTexture = first_texture;
}

void BloomExtractEffect::SetConstants(f32 threshold) {
    m_threshold = threshold;
    m_dirtyFlags |= DirtyConstantBuffer;
}

void BloomExtractEffect::Apply(ID3D12GraphicsCommandList* command_list) {
    //NOTE(sean): update dirty data
    if (m_dirtyFlags & DirtyConstantBuffer) {
        auto constant_buffer = GraphicsMemory::Get(m_device.Get()).AllocateConstant<BloomExtractConstants>();

        BloomExtractConstants data = {
            m_threshold
        };

        memcpy((u8*)constant_buffer.Memory(), &data, sizeof(BloomExtractConstants));
        std::swap(m_constantBuffer, constant_buffer);

        m_dirtyFlags &= ~DirtyConstantBuffer;
    }

    //NOTE(sean): set root signature and parameters
    command_list->SetGraphicsRootSignature(m_rootSignature.Get());

    //NOTE(sean): set render resources
    command_list->SetGraphicsRootDescriptorTable(0, m_firstTexture);

    //NOTE(sean): set render constants
    command_list->SetGraphicsRootConstantBufferView(1, m_constantBuffer.GpuAddress());

    //NOTE(sean): set pipeline state
    command_list->SetPipelineState(m_pso.Get());
}

// BLOOM BLUR

namespace {
    struct __declspec(align(16)) BloomBlurConstants {
        f32 intensity;
        f32 scale;
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

void BloomBlurEffect::SetTextures(D3D12_GPU_DESCRIPTOR_HANDLE first_texture) {
    m_firstTexture = first_texture;
}

void BloomBlurEffect::SetConstants(f32 intensity, f32 scale) {
    m_intensity = intensity;
    m_scale = scale;
    m_dirtyFlags |= DirtyConstantBuffer;
}

void BloomBlurEffect::Apply(ID3D12GraphicsCommandList* command_list) {
    //NOTE(sean): update dirty data
    if (m_dirtyFlags & DirtyConstantBuffer) {
        auto constant_buffer = GraphicsMemory::Get(m_device.Get()).AllocateConstant<BloomBlurConstants>();

        BloomBlurConstants data = {
            m_intensity,
            m_scale
        };

        memcpy((u8*)constant_buffer.Memory(), &data, sizeof(BloomBlurConstants));
        std::swap(m_constantBuffer, constant_buffer);

        m_dirtyFlags &= ~DirtyConstantBuffer;
    }

    //NOTE(sean): set root signature and parameters
    command_list->SetGraphicsRootSignature(m_rootSignature.Get());

    //NOTE(sean): set render resources
    command_list->SetGraphicsRootDescriptorTable(0, m_firstTexture);

    //NOTE(sean): set render constants
    command_list->SetGraphicsRootConstantBufferView(1, m_constantBuffer.GpuAddress());

    //NOTE(sean): set pipeline state
    command_list->SetPipelineState(m_pso.Get());
}

// BLOOM COMBINE

namespace {
    struct __declspec(align(16)) BloomCombineConstants {
        f32 scale;
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
    CreatePostProcessPassData(device, pipeline_state_desc, m_rootSignature, m_pso, 2, L"BloomCombine_PS.cso");
}

void BloomCombineEffect::SetTextures(D3D12_GPU_DESCRIPTOR_HANDLE first_texture) {
    m_firstTexture = first_texture;
}

void BloomCombineEffect::SetConstants(f32 scale) {
    m_scale = scale;
    m_dirtyFlags |= DirtyConstantBuffer;
}

void BloomCombineEffect::Apply(ID3D12GraphicsCommandList* command_list) {
    //NOTE(sean): update dirty data
    if (m_dirtyFlags & DirtyConstantBuffer) {
        auto constant_buffer = GraphicsMemory::Get(m_device.Get()).AllocateConstant<BloomCombineConstants>();

        BloomCombineConstants data = {
            m_scale
        };

        memcpy((u8*)constant_buffer.Memory(), &data, sizeof(BloomCombineConstants));
        std::swap(m_constantBuffer, constant_buffer);

        m_dirtyFlags &= ~DirtyConstantBuffer;
    }

    //NOTE(sean): set root signature and parameters
    command_list->SetGraphicsRootSignature(m_rootSignature.Get());

    //NOTE(sean): set render resources
    command_list->SetGraphicsRootDescriptorTable(0, m_firstTexture);

    //NOTE(sean): set render constants
    command_list->SetGraphicsRootConstantBufferView(1, m_constantBuffer.GpuAddress());

    //NOTE(sean): set pipeline state
    command_list->SetPipelineState(m_pso.Get());
}
