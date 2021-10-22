#ifndef TONEMAP_EFFECT_H
#define TONEMAP_EFFECT_H

#include "LightingEffect.h"
#include <Effects.h>
#include <Renderer3D.h>
#include <vector>

class TonemapEffect : public DirectX::IEffect {
public:
    TonemapEffect(
        ID3D12Device* device,
        const DirectX::EffectPipelineStateDescription& pipeline_state_desc
    );

    void Apply(ID3D12GraphicsCommandList* command_list) override;
    void SetTextures(DescriptorHeap* textures);
    void UpdateConstants(
        u32 width,
        u32 height,
        Vec3 tint,
        f32 glasses,
        f32 saturation
    );

private:
    u32 DimensionsX;
    u32 DimensionsY;
    f32 TintR;
    f32 TintG;
    f32 TintB;
    f32 Glasses;
    f32 Saturation;

    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;

    u32 m_dirtyFlags;
    DirectX::GraphicsResource m_constantBuffer;

    D3D12_GPU_DESCRIPTOR_HANDLE m_colorTexture;
};

#endif
