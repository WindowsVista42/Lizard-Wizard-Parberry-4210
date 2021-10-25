#ifndef POST_PROCESS_EFFECT_H
#define POST_PROCESS_EFFECT_H

#include "Defines.h"
#include <Effects.h>
#include <Renderer3D.h>
#include <vector>

namespace PPDescriptors { enum e {
    InputSRVs, ConstantBuffer,
    Count
};}

class LightingEffect: public DirectX::IEffect {
public:
    LightingEffect(
        ID3D12Device* device,
        const DirectX::EffectPipelineStateDescription& pipeline_state_desc
    );

    void Apply(ID3D12GraphicsCommandList* command_list) override;
    void SetTextures(DescriptorHeap* textures);

    void SetLightCount(u32 light_count);
    void SetLightPosition(Vec4 position, u32 index);
    void SetLightColor(Vec4 color, u32 index);

private:
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;

    u32 m_dirtyFlags;
    DirectX::GraphicsResource m_constantBuffer;

    D3D12_GPU_DESCRIPTOR_HANDLE m_colorTexture;
    D3D12_GPU_DESCRIPTOR_HANDLE m_normalTexture;
    D3D12_GPU_DESCRIPTOR_HANDLE m_positionTexture;

    u32 light_count;
    Vec4 light_positions[254];
    Vec4 light_colors[254];
};

#endif
