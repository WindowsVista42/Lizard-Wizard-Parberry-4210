#ifndef POST_PROCESS_EFFECT_H
#define POST_PROCESS_EFFECT_H

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
    void SetTextures(
        D3D12_GPU_DESCRIPTOR_HANDLE color_texture,
        D3D12_GPU_DESCRIPTOR_HANDLE normal_texture,
        D3D12_GPU_DESCRIPTOR_HANDLE position_texture
    );

private:
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;

    u32 m_dirtyFlags;
    DirectX::GraphicsResource m_constantBuffer;

    D3D12_GPU_DESCRIPTOR_HANDLE m_colorTexture;
    D3D12_GPU_DESCRIPTOR_HANDLE m_normalTexture;
    D3D12_GPU_DESCRIPTOR_HANDLE m_positionTexture;
};

#endif
