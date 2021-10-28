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
    float2(-0.002,  0.002),
    float2( 0.000,  0.002),
    float2( 0.002,  0.002),
    float2(-0.002,  0.000),
    float2( 0.000,  0.000),
    float2( 0.002,  0.000),
    float2(-0.002, -0.002),
    float2( 0.000, -0.002),
    float2( 0.002, -0.002)
};

[RootSignature(RS)]
PixelOutput main(VertexOutput input) {
    PixelOutput output;

    float3 pixel_color = Color.Sample(Sampler, input.Texture).rgb;
    float3 pixel_normal = Normal.Sample(Sampler, input.Texture).xyz;
    float3 pixel_position = Position.Sample(Sampler, input.Texture).xyz;

    /*
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

            const float x = dot(pixel_normal, sample_normal);
            const float normal_weight = x;//pow(cos(PI * x / 2.0), 0.5);

            //ao_strength -= (position_weight * normal_weight) / (DIVI);
            ao_strength -= normal_weight / (DIVI);
        }
    */

    const float3 camera_position = float3(camera_x, camera_y, camera_z);
    float ao_strength = 1.0f;
    //float3 diff1 = float3(0,0,0);
    float3 diff2 = float3(0,0,0);
    for (uint x = 0; x < 9; x += 1) {
        const float2 sample_uv = samples[x];
    	const float3 sample_position = Position.Sample(Sampler, input.Texture + sample_uv).xyz;
        //const float3 position_diff = camera_position - sample_position;
    	const float3 sample_normal = Normal.Sample(Sampler, input.Texture + sample_uv).xyz;

        //diff1 += sample_position * kernel[x];
        diff2 += sample_normal * kernel[x];
    }

    //ao_strength -= dot(diff1, diff1) / 128.0f;//clamp(dot(diff2, diff2), 1.0, 32.0) / 128.0;

    //if (dot(diff1, diff1) > 128.0f) {
    //    ao_strength -= 0.2f;
    //}

    if (dot(diff2, diff2) > 1.0f) {
        ao_strength -= 0.2f;
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