#include "LightingEffect.h"
#include "../Renderer.h"
#include <ReadData.h>
#include "../Defines.h"

//NOTE(sean): this is actually a really nice way to do a bitset,
//courtesy of --> https://github.com/microsoft/DirectXTK12/wiki/Authoring-an-Effect
namespace {
    struct __declspec(align(16)) GameEffectConstants {
        u32 DimensionsX;
        u32 DimensionsY;
        f32 TintR;
        f32 TintG;
        f32 TintB;
        f32 Glasses;
        f32 Saturation;
    };

    static_assert((sizeof(GameEffectConstants) % 16) == 0, "Constant Buffer size alignment");

    constexpr u32 DirtyConstantBuffer = 0x1;
    //NOTE(sean): other bit flags go here
}

//TODO(sean): update this to the right implementation
TonemapEffect::TonemapEffect(
    ID3D12Device* device,
    const DirectX::EffectPipelineStateDescription& pipeline_state_desc
) :
    m_device(device),
    m_dirtyFlags(u32(-1))
{
    CreatePostProcessPassData(device, pipeline_state_desc, m_rootSignature, m_pso, 1, L"Tonemap_PS.cso");
}

void TonemapEffect::SetTextures(D3D12_GPU_DESCRIPTOR_HANDLE first_texture) {
    m_firstTexture = first_texture;
}

void TonemapEffect::UpdateConstants(
    u32 width,
    u32 height,
    Vec3 tint,
    f32 glasses,
    f32 saturation
) {
    DimensionsX = width;
    DimensionsY = height;
    TintR = tint.x;
    TintG = tint.y;
    TintB = tint.z;
    Glasses = glasses * (f32)height; // Normalize to pixels
    Saturation = powf(saturation, 2.2f); // Gamma correction type thing to get a more "linear" feeling response

    m_dirtyFlags |= DirtyConstantBuffer;
}

//TODO(sean): update this to the right implementation
void TonemapEffect::Apply(ID3D12GraphicsCommandList* command_list) {
    //NOTE(sean): update dirty data
    if (m_dirtyFlags & DirtyConstantBuffer) {
        auto constant_buffer = GraphicsMemory::Get(m_device.Get()).AllocateConstant<GameEffectConstants>();

        GameEffectConstants data = {
            DimensionsX,
            DimensionsY,
            TintR,
            TintG,
            TintB,
            Glasses,
            Saturation
        };

        memcpy(constant_buffer.Memory(), &data, constant_buffer.Size());
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
