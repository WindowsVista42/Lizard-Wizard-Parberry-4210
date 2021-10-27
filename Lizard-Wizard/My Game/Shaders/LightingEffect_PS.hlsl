#include "LightingEffect_Common.hlsli"
#include "PostProcess_Common.hlsli"

static const float PI = 3.14159265f;

static const int AO_SAMPLE_COUNT = 1;
static const float AO_TOTAL_SAMPLE_DISTANCE = 0.02f;
static const float AO_SAMPLE_DISTANCE = AO_TOTAL_SAMPLE_DISTANCE / float(AO_SAMPLE_COUNT * 2 + 1);
static const float DIVI = float((AO_SAMPLE_COUNT * 2 + 1) * (AO_SAMPLE_COUNT * 2 + 1));

[RootSignature(LightingEffectRS)]
PixelOutput main(VertexOutput input) {
    PixelOutput output;

    float3 pixel_color = Color.Sample(Sampler, input.Texture).rgb;
    float3 pixel_normal = Normal.Sample(Sampler, input.Texture).xyz;
    float3 pixel_position = Position.Sample(Sampler, input.Texture).xyz;

    // Calculate Ambient Occlusion
    float ao_strength = 1.0f;
    for (int x = -AO_SAMPLE_COUNT; x < AO_SAMPLE_COUNT; x += 1) {
        const float xf = float(x);
        for (int y = -AO_SAMPLE_COUNT; y < AO_SAMPLE_COUNT; y += 1) {
            const float yf = float(y);
            const float2 sample_uv = float2(xf * AO_SAMPLE_DISTANCE, yf * AO_SAMPLE_DISTANCE);

            const float3 sample_position = Position.Sample(Sampler, input.Texture + sample_uv).xyz;
            const float3 sample_normal = Normal.Sample(Sampler, input.Texture + sample_uv).xyz;

            const float3 position_diff = pixel_position - sample_position;
            const float position_weight = 1.0f / clamp(dot(position_diff, position_diff), 1.0f, 4.0f);

            const float x = -dot(pixel_normal, sample_normal);
            const float normal_weight = clamp((1.0f - pow(abs(x), 8.0)), -1.0, 1.0);

            ao_strength -= (position_weight * normal_weight) / (DIVI + 16.0f);
        }
    }

    // Calculate Lighting
    float3 output_color = float3(0.0f, 0.0f, 0.0f);
    for (uint index = 0; index < light_count; index += 1) {
        const float3 LIGHT_POSITION = Lights[index].Position.xyz;
        const float3 LIGHT_COLOR = Lights[index].Color.rgb;

        const float3 position_diff = LIGHT_POSITION - pixel_position;
        const float light_strength = 1.0f / sqrt(dot(position_diff, position_diff));
        const float3 light_direction = normalize(position_diff);
        const float dotprod = dot(pixel_normal, light_direction);
        const float3 shape = clamp((dotprod + 2.0) / 2.0, 0.0, 1.0);
        const float3 light_color = LIGHT_COLOR * light_strength * shape;

        output_color += light_color * pixel_color;
    }

    //const float3 AMBIENT = float3(0.02, 0.02, 0.08);
    output.Color = float4(output_color * ao_strength, 1.0f);

    return output;
}