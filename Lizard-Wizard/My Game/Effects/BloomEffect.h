#ifndef BLOOMEXTRACT_H
#define BLOOMEXTRACT_H

#include "../Defines.h"
#include "EffectsCommon.h"

class BloomExtractEffect : public DirectX::IEffect {
public:
    BloomExtractEffect(
        ID3D12Device* device,
        const DirectX::EffectPipelineStateDescription& pipeline_state_desc
    );

    void Apply(ID3D12GraphicsCommandList* command_list) override;
    void SetTextures(D3D12_GPU_DESCRIPTOR_HANDLE first_texture);
    void SetConstants(f32 threshold);

private:
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;

    u32 m_dirtyFlags;
    DirectX::GraphicsResource m_constantBuffer;

    f32 m_threshold;
    D3D12_GPU_DESCRIPTOR_HANDLE m_firstTexture;
};

//#endif

//#ifndef BLOOMBLUR_H
//#define BLOOMBLUR_H

//#include "Defines.h"

class BloomBlurEffect : public DirectX::IEffect {
public:
    BloomBlurEffect(
        ID3D12Device* device,
        const DirectX::EffectPipelineStateDescription& pipeline_state_desc
    );

    void Apply(ID3D12GraphicsCommandList* command_list) override;
    void SetTextures(D3D12_GPU_DESCRIPTOR_HANDLE first_texture);
    void SetConstants(f32 intensity, f32 scale);

private:
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;

    u32 m_dirtyFlags;
    DirectX::GraphicsResource m_constantBuffer;

    f32 m_intensity;
    f32 m_scale;
    D3D12_GPU_DESCRIPTOR_HANDLE m_firstTexture;
};

//#endif

//#ifndef BLOOMCOMBINE_H
//#define BLOOMCOMBINE_H

//#include "Defines.h"

class BloomCombineEffect : public DirectX::IEffect {
public:
    BloomCombineEffect(
        ID3D12Device* device,
        const DirectX::EffectPipelineStateDescription& pipeline_state_desc
    );

    void Apply(ID3D12GraphicsCommandList* command_list) override;
    void SetTextures(D3D12_GPU_DESCRIPTOR_HANDLE first_texture);
    void SetConstants(f32 scale);

private:
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;

    u32 m_dirtyFlags;
    DirectX::GraphicsResource m_constantBuffer;

    f32 m_scale;
    D3D12_GPU_DESCRIPTOR_HANDLE m_firstTexture;
};

#endif
