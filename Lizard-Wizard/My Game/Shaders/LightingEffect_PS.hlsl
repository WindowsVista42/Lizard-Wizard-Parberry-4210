#include "LightingEffect_Common.hlsli"
#include "PostProcess_Common.hlsli"

[RootSignature(LightingEffectRS)]
PixelOutput main(VertexOutput input) {
    PixelOutput output;

    float3 pixel_color = Color.Sample(Sampler, input.Texture).rgb;
    float3 pixel_normal = Normal.Sample(Sampler, input.Texture).xyz;
    float3 pixel_position = Position.Sample(Sampler, input.Texture).xyz;

    float3 output_color = float3(0.0f, 0.0f, 0.0f);
    for (uint index = 0; index < light_count; index += 1) {
        const float3 LIGHT_POSITION = Lights[index].Position.xyz;
        const float3 LIGHT_COLOR = Lights[index].Color.rgb;

        float3 position_diff = LIGHT_POSITION - pixel_position;
        float light_strength = 1.0f / sqrt(dot(position_diff, position_diff));
        float3 light_direction = normalize(position_diff);
        float3 light_color = LIGHT_COLOR * light_strength * clamp((dot(pixel_normal, light_direction) + 1.1) * 0.5, 0.0, 1.0);
        output_color += light_color * pixel_color;
    }

    //const float3 AMBIENT = float3(0.02, 0.02, 0.08);
    output.Color = float4(output_color, 1.0f);

    return output;
}