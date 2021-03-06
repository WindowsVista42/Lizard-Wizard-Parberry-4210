#ifndef GAMEEFFECT_H
#define GAMEFFECT_H

#include "../Defines.h"
#include <Effects.h>
#include <Renderer3D.h>

//NOTE(sean): Reference --> https://github.com/microsoft/DirectXTK12/wiki/Authoring-an-Effect
class DeferredEffect:
    public DirectX::IEffect,
    public DirectX::IEffectMatrices
{
public:
    DeferredEffect(
        ID3D12Device* device,
        const DirectX::EffectPipelineStateDescription& pipeline_state_desc
    );

    void Apply(ID3D12GraphicsCommandList* command_list) override;

    void XM_CALLCONV SetWorld(DirectX::FXMMATRIX world) override;
    void XM_CALLCONV SetView(DirectX::FXMMATRIX view) override;
    void XM_CALLCONV SetProjection(DirectX::FXMMATRIX projection) override;
    void XM_CALLCONV SetMatrices(DirectX::FXMMATRIX world, DirectX::CXMMATRIX view, DirectX::CXMMATRIX projection) override;

    void SetGlow(Vec3 glow);

    //TODO(sean): state setting functions
    //Something to set textures, idk
    void SetTextures(D3D12_GPU_DESCRIPTOR_HANDLE first_texture);
private:
    enum Descriptors {
        InputSRV,
        ConstantBuffer,
        Count
    };

    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;

    u32 m_dirtyFlags;
    XMMATRIX m_world;
    XMMATRIX m_transposeWorld;
    XMMATRIX m_view;
    XMMATRIX m_projection;
    XMMATRIX m_worldViewProjection;
    Vec4 m_glow;
    DirectX::GraphicsResource m_constantBuffer;

    D3D12_GPU_DESCRIPTOR_HANDLE m_firstTexture;
};

#endif
