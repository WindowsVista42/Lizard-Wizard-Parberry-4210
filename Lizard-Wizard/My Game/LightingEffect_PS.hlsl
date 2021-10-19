#include "LightingEffect_Common.hlsli"
#include "PostProcess_Common.hlsli"

[RootSignature(LightingEffectRS)]
PixelOutput main(VertexOutput input) {
    PixelOutput output;

    Light GLOBAL_LIGHT;
    GLOBAL_LIGHT.Position = float3(0.0f, 200.0f, 0.0f);
    GLOBAL_LIGHT.Color = float3(2.0f, 2.0f, 2.0f);

    float3 pixel_color = Color.Sample(Sampler, input.Texture).rgb;
    float3 pixel_normal = Normal.Sample(Sampler, input.Texture).xyz;
    float3 pixel_position = Position.Sample(Sampler, input.Texture).xyz;

    float3 light_direction = normalize(GLOBAL_LIGHT.Position - pixel_position);
    float3 light_color = GLOBAL_LIGHT.Color * ((dot(pixel_normal, light_direction) + 1.0f) * 0.5f);

    output.Color = float4(light_color * pixel_color, 1.0f);

    return output;
}