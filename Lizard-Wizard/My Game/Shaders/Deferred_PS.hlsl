#include "Deferred_Common.hlsli"

// Passthrough to G-Buffer
[RootSignature(DeferredEffectRS)]
PixelOutput main(VertexOutput input) {
    PixelOutput output;

    float4 diffuse = Color.Sample(Sampler, input.Texture);
    if (SolidColor.a != 0.0) {
        output.Diffuse = float4(SolidColor.rgb * SolidColor.a, 1.0f);
    }
    output.Normal = float4(input.Normal.xyz, 1.0f);
    output.Position = input.Position;

    return output;
}