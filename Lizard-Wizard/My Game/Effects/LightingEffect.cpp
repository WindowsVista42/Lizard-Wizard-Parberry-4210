#include "LightingEffect.h"
#include "EffectsCommon.h"
#include <ReadData.h>
#include <iostream>

//NOTE(sean): this is actually a really nice way to do a bitset,
//courtesy of --> https://github.com/microsoft/DirectXTK12/wiki/Authoring-an-Effect
namespace {
    struct __declspec(align(16)) GameEffectConstants {
        u32 light_count;
        f32 camera_x;
        f32 camera_y;
        f32 camera_z;
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
    CreatePostProcessPassData(device, pipeline_state_desc, m_rootSignature, m_pso, 3, L"Lighting_PS.cso");
}

void LightingEffect::SetTextures(D3D12_GPU_DESCRIPTOR_HANDLE first_texture) {
    m_firstTexture = first_texture;
}

void LightingEffect::SetLightCount(u32 light_count) {
    this->light_count = light_count;
    m_dirtyFlags |= DirtyConstantBuffer;
}

void LightingEffect::SetCameraPosition(Vec3 camera_position) {
    this->camera_position = camera_position;
    m_dirtyFlags |= DirtyConstantBuffer;
}

void LightingEffect::SetLight(usize index, Light light) {
    lights[index] = light;
    m_dirtyFlags |= DirtyConstantBuffer;
}

Light* LightingEffect::Lights() {
    m_dirtyFlags |= DirtyConstantBuffer;
    return lights;
}

//TODO(sean): update this to the right implementation
void LightingEffect::Apply(ID3D12GraphicsCommandList* command_list) {
    //NOTE(sean): update dirty data
    if (m_dirtyFlags & DirtyConstantBuffer) {
        auto constant_buffer = GraphicsMemory::Get(m_device.Get()).AllocateConstant<GameEffectConstants>();

        GameEffectConstants data = {
            light_count,
            camera_position.x,
            camera_position.y,
            camera_position.z
        };

        memcpy((u8*)constant_buffer.Memory(), &data, sizeof(u32) * 4);
        memcpy((u8*)constant_buffer.Memory() + offsetof(GameEffectConstants, lights), lights, sizeof(Light) * _countof(lights));
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
