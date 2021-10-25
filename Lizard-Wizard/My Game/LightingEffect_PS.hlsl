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
        const float3 LIGHT_POSITION = LightPos[index].xyz;
        const float3 LIGHT_COLOR = LightCol[index].rgb;

        //const float3 LIGHT_POSITION = LightPos[index].xyz;
        //const float3 LIGHT_COLOR = LightCol[index].rgb;

        //const Light light = Lights[index];
        //const float3 LIGHT_POSITION = light.Position.xyz;
        //const float3 LIGHT_COLOR = light.Color.xyz;

        //const float3 LIGHT_POSITION = Lights[2*index + 0].Position.xyz;
        //const float3 LIGHT_COLOR = Lights[2*index + 1].Color.xyz;

        //const float3 LIGHT_POSITION = float3(0.0f, 200.0f, 0.0f);
        //const float3 LIGHT_COLOR = float3(8000.0f, 2000.0f, 2000.0f);

        float3 position_diff = LIGHT_POSITION - pixel_position;
        float light_strength = 1.0f / sqrt(dot(position_diff, position_diff));
        float3 light_direction = normalize(position_diff);
        float3 light_color = LIGHT_COLOR * light_strength * ((dot(pixel_normal, light_direction) + 1.0f) * 0.5f);
        output_color += light_color * pixel_color;
    }

    output.Color = float4(output_color, 1.0f);

    return output;
}