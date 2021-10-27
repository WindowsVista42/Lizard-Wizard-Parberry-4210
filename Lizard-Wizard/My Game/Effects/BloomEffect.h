#ifndef BLOOMEXTRACT_H
#define BLOOMEXTRACT_H

#include "Defines.h"
#include "EffectsCommon.h"

class BloomExtractEffect : public DirectX::IEffect {
    BloomExtractEffect(
        ID3D12Device* device,
        const DirectX::EffectPipelineStateDescription& pipeline_state_desc
    );

    void Apply(ID3D12GraphicsCommandList* command_list) override;
    void SetTextures(DescriptorHeap* textures);
private:
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;

    u32 m_dirtyFlags;
    DirectX::GraphicsResource m_constantBuffer;

    D3D12_GPU_DESCRIPTOR_HANDLE m_firstTexture;
};

//#endif

//#ifndef BLOOMBLUR_H
//#define BLOOMBLUR_H

//#include "Defines.h"

class BloomBlurEffect : public DirectX::IEffect {
    BloomBlurEffect(
        ID3D12Device* device,
        const DirectX::EffectPipelineStateDescription& pipeline_state_desc
    );

    void Apply(ID3D12GraphicsCommandList* command_list) override;
    void SetTextures(DescriptorHeap* textures);

private:
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;

    u32 m_dirtyFlags;
    DirectX::GraphicsResource m_constantBuffer;

    D3D12_GPU_DESCRIPTOR_HANDLE m_firstTexture;
};

//#endif

//#ifndef BLOOMCOMBINE_H
//#define BLOOMCOMBINE_H

//#include "Defines.h"

class BloomCombineEffect : public DirectX::IEffect {
    BloomCombineEffect(
        ID3D12Device* device,
        const DirectX::EffectPipelineStateDescription& pipeline_state_desc
    );

    void Apply(ID3D12GraphicsCommandList* command_list) override;
    void SetTextures(DescriptorHeap* textures);

private:
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;

    u32 m_dirtyFlags;
    DirectX::GraphicsResource m_constantBuffer;

    D3D12_GPU_DESCRIPTOR_HANDLE m_firstTexture;
};

#endif
