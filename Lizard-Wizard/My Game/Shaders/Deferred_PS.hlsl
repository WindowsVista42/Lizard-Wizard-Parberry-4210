#include "Deferred_Common.hlsli"

// Passthrough to G-Buffer
[RootSignature(DeferredEffectRS)]
PixelOutput main(VertexOutput input) {
    PixelOutput output;

    output.Diffuse = Color.Sample(Sampler, input.Texture) * Glow;
    output.Normal = float4(input.Normal.xyz, 1.0f);
    output.Position = input.Position;

    return output;
}