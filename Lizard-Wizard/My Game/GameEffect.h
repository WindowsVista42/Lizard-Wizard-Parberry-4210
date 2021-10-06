#ifndef GAMEEFFECT_H
#define GAMEFFECT_H

#include <Effects.h>
#include <Renderer3D.h>

//NOTE(sean): Reference --> https://github.com/microsoft/DirectXTK12/wiki/Authoring-an-Effect
class GameEffect: public DirectX::IEffect {
public:
    GameEffect(
        ID3D12Device* device,
        const DirectX::EffectPipelineStateDescription& pipeline_state_desc
    );

    void Apply(ID3D12GraphicsCommandList* command_list) override;

    //TODO(sean): state setting functions

private:
    enum Descriptors {
        ConstantBuffer,
        Count
    };

    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;

    //TODO(sean): other state
};

#endif
