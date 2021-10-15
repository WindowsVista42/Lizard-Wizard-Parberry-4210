#include "GameEffect_Common.hlsli"

// Passthrough to G-Buffer
[RootSignature(GameEffectRS)]
PixelOutput main(VertexOutput input) {
    PixelOutput output;

    Light GLOBAL_LIGHT;
    GLOBAL_LIGHT.Position = float3(0.0f, 200.0f, 0.0f);
    GLOBAL_LIGHT.Color = float3(1.0f, 1.0f, 1.0f);

    float3 pixel_position = input.Position.xyz;
    float3 pixel_normal = input.Normal;
    float3 light_direction = normalize(GLOBAL_LIGHT.Position - pixel_position);
    float light_strength = (dot(pixel_normal, light_direction) + 1.0f) * 0.5f;

    //output.Diffuse = float4(light_strength, light_strength, light_strength, 1.0f);

    //output.Diffuse = float4(Texture.Sample(Sampler, input.Texture));
    output.Diffuse = float4(input.Texture, 1.0f, 1.0f);
    output.Normal = float4(input.Normal.xyz, 1.0f);
    output.Position = input.Position;

    return output;
}