#ifndef POST_PROCESS_EFFECT_H
#define POST_PROCESS_EFFECT_H

#include <Effects.h>
#include <Renderer3D.h>

class PostProcessEffect: public DirectX::IEffect {
public:
    PostProcessEffect(
        ID3D12Device* device,
        const DirectX::EffectPipelineStateDescription& pipeline_state_desc
    );

    void Apply(ID3D12GraphicsCommandList* command_list) override;

private:
    enum Descriptors {
        ConstantBuffer,
        Count
    };

    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;

    u32 m_dirtyFlags;
    DirectX::GraphicsResource m_constantBuffer;

    //TODO(sean): other state
};

#endif
