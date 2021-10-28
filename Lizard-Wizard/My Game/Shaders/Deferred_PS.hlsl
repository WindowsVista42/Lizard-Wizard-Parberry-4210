#include "Deferred_Common.hlsli"

// Passthrough to G-Buffer
[RootSignature(DeferredEffectRS)]
PixelOutput main(VertexOutput input) {
    PixelOutput output;

    //output.Diffuse = Color.Sample(Sampler, input.Texture);
    output.Diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
    output.Normal = float4(input.Normal.xyz, 1.0f);
    output.Position = input.Position;

    return output;
}