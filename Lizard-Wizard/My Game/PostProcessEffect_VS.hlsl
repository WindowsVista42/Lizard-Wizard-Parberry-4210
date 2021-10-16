#include "PostProcessEffect_Common.hlsli"

[RootSignature(PostProcessEffectRS)]
VertexOutput main(VertexInput input) {
    VertexOutput output;
    output.VertexPosition = float4(0.0f, 0.0f, 0.0f, 1.0f);
    return output;
}