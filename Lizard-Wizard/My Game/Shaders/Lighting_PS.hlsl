#include "PostProcess_Common.hlsli"

#define RS \
"RootFlags ("\
"    DENY_DOMAIN_SHADER_ROOT_ACCESS |"\
"    DENY_GEOMETRY_SHADER_ROOT_ACCESS |"\
"    DENY_HULL_SHADER_ROOT_ACCESS ),"\
"DescriptorTable(SRV(t0), visibility = SHADER_VISIBILITY_PIXEL)," \
"DescriptorTable(SRV(t1), visibility = SHADER_VISIBILITY_PIXEL)," \
"DescriptorTable(SRV(t2), visibility = SHADER_VISIBILITY_PIXEL)," \
"CBV(b0, visibility = SHADER_VISIBILITY_PIXEL)," \
"StaticSampler(s0," \
"    filter = FILTER_MIN_MAG_MIP_LINEAR," \
"    addressU = TEXTURE_ADDRESS_CLAMP," \
"    addressV = TEXTURE_ADDRESS_CLAMP," \
"    addressW = TEXTURE_ADDRESS_CLAMP," \
"    visibility = SHADER_VISIBILITY_PIXEL)"

Texture2D<float4> Color : register(t0);
Texture2D<float4> Normal : register(t1);
Texture2D<float4> Position : register(t2);

struct Light {
    float4 Position;
    float4 Color;
};

sampler Sampler : register(s0);

cbuffer GameEffectConstants : register(b0) {
    uint light_count;
    float camera_x;
    float camera_y;
    float camera_z;
    Light Lights[254];
}

//cbuffer LightPositions : register(b1) {
//}
//
//cbuffer LightColors : register(b2) {
//}

struct PixelOutput {
    float4 Color : SV_TARGET0;
};

static const float PI = 3.14159265f;

static const int AO_SAMPLE_COUNT = 1;
static const float AO_TOTAL_SAMPLE_DISTANCE = 0.02f;
static const float AO_SAMPLE_DISTANCE = AO_TOTAL_SAMPLE_DISTANCE / float(AO_SAMPLE_COUNT * 2 + 1);
static const float DIVI = float((AO_SAMPLE_COUNT * 2 + 1) * (AO_SAMPLE_COUNT * 2 + 1));

static const float kernel[9] = { -1.0, -1.0, -1.0, -1.0, 8.0, -1.0, -1.0, -1.0, -1.0 };
static const float2 samples[9] = {
    float2(-0.001,  0.001),
    float2( 0.000,  0.001),
    float2( 0.001,  0.001),
    float2(-0.001,  0.000),
    float2( 0.000,  0.000),
    float2( 0.001,  0.000),
    float2(-0.001, -0.001),
    float2( 0.000, -0.001),
    float2( 0.001, -0.001)
};

[RootSignature(RS)]
PixelOutput main(VertexOutput input) {
    PixelOutput output;

    const float3 pixel_color = Color.Sample(Sampler, input.Texture).rgb;
    const float3 pixel_normal = Normal.Sample(Sampler, input.Texture).xyz;
    const float3 pixel_position = Position.Sample(Sampler, input.Texture).xyz;
    const float3 camera_position = float3(camera_x, camera_y, camera_z);

    const float3 pixel_camera_diff = pixel_position - camera_position;
    const float pixel_camera_dist2 = dot(pixel_camera_diff, pixel_camera_diff);
    const float scl = 0.04;

    float ao_strength = 1.0f;
    for (uint x = 0; x < 9; x += 1) {
        const float2 sample_uv = samples[x];
    	const float3 sample_position = Position.Sample(Sampler, input.Texture + sample_uv).xyz;
    	const float3 sample_normal = Normal.Sample(Sampler, input.Texture + sample_uv).xyz;

    	const float3 sample_camera_diff = sample_position - camera_position;
    	const float sample_camera_dist2 = dot(sample_camera_diff, sample_camera_diff);

        const float z = dot(sample_normal, pixel_normal);
        ao_strength -= (1.0 - pow(abs(z), 4.0)) * scl;

        const float distance_div = sample_camera_dist2 / pixel_camera_dist2;
        if (distance_div > 1.5) {
            ao_strength -= (scl / 4.0);
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