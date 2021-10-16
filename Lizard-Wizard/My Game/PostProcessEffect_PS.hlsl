#include "PostProcessEffect_Common.hlsli"

// Passthrough to G-Buffer
[RootSignature(PostProcessEffectRS)]
PixelOutput main(VertexOutput input) {
    PixelOutput output;

    Light GLOBAL_LIGHT;
    GLOBAL_LIGHT.Position = float3(0.0f, 200.0f, 0.0f);
    GLOBAL_LIGHT.Color = float3(1.0f, 1.0f, 1.0f);

    float3 pixel_color = Diffuse.Sample(Sampler, input.Texture).rgb;
    float3 pixel_normal = Normal.Sample(Sampler, input.Texture).xyz;
    float3 pixel_position = Position.Sample(Sampler, input.Texture).xyz;

    float3 light_direction = normalize(GLOBAL_LIGHT.Position - pixel_position);
    float light_strength = (dot(pixel_normal, light_direction) + 1.0f) * 0.5f;

    output.BackBuffer = float4(float3(light_strength * pixel_color), 1.0f);

    return output;
}