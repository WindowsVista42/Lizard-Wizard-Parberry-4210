#include "../Defines.h"

void CreatePostProcessPassData(
    ID3D12Device* device,
    const DirectX::EffectPipelineStateDescription& pipeline_state_desc,
    Microsoft::WRL::ComPtr<ID3D12RootSignature>& m_rootSignature,
    Microsoft::WRL::ComPtr<ID3D12PipelineState>& m_pso,
    const u32 input_texture_count,
    const wchar* pixel_shader
);