#include "PostProcessEffect_Common.hlsli"

// Passthrough to G-Buffer
[RootSignature(PostProcessEffectRS)]
PixelOutput main(VertexOutput input) {
    PixelOutput output;
    return output;
}