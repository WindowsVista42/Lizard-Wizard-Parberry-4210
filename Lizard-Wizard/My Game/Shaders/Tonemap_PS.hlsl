#include "PostProcess_Common.hlsli"

#define RS \
"RootFlags ("\
"    DENY_DOMAIN_SHADER_ROOT_ACCESS |"\
"    DENY_GEOMETRY_SHADER_ROOT_ACCESS |"\
"    DENY_HULL_SHADER_ROOT_ACCESS ),"\
"DescriptorTable(SRV(t0), visibility = SHADER_VISIBILITY_PIXEL)," \
"CBV(b0, visibility = SHADER_VISIBILITY_PIXEL)," \
"StaticSampler(s0," \
"    filter = FILTER_MIN_MAG_MIP_LINEAR," \
"    addressU = TEXTURE_ADDRESS_CLAMP," \
"    addressV = TEXTURE_ADDRESS_CLAMP," \
"    addressW = TEXTURE_ADDRESS_CLAMP," \
"    visibility = SHADER_VISIBILITY_PIXEL)"

Texture2D<float4> Color : register(t0);

sampler Sampler : register(s0);

cbuffer Constants : register(b0) {
    uint DimensionsX;
    uint DimensionsY;
    float TintR;
    float TintG;
    float TintB;
    float Glasses;
    float Saturation;
}

struct PixelOutput {
    float4 Color : SV_TARGET0;
};

// https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
static const float3x3 ACESInputMat =
{
    {0.59719, 0.35458, 0.04823},
    {0.07600, 0.90834, 0.01566},
    {0.02840, 0.13383, 0.83777}
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
static const float3x3 ACESOutputMat =
{
    { 1.60475, -0.53108, -0.07367},
    {-0.10208,  1.10813, -0.00605},
    {-0.00327, -0.07276,  1.07602}
};

float3 RRTAndODTFit(float3 v)
{
    float3 a = v * (v + 0.0245786f) - 0.000090537f;
    float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

float3 ACESFitted(float3 color)
{
    color = mul(ACESInputMat, color);

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = mul(ACESOutputMat, color);

    // Clamp to [0, 1]
    color = saturate(color);

    return color;
}

//

float Luminance(float3 color) {
    return dot(color, float3(0.299f, 0.587f, 0.114f));
}

float3 BoxBlur3x3(uint2 dim, float2 uv, float intensity) {
    float2 d = (float2(1.0f, 1.0f) / dim) * intensity;
    const float kernel = 1.0f / 9.0f;

    float3 output;
    output  = kernel * Color.Sample(Sampler, float2(uv.x - d.x, uv.y - d.y)).xyz;
    output += kernel * Color.Sample(Sampler, float2(uv.x      , uv.y - d.y)).xyz;
    output += kernel * Color.Sample(Sampler, float2(uv.x + d.x, uv.y - d.y)).xyz;

    output += kernel * Color.Sample(Sampler, float2(uv.x - d.x, uv.y      )).xyz;
    output += kernel * Color.Sample(Sampler, float2(uv.x      , uv.y      )).xyz;
    output += kernel * Color.Sample(Sampler, float2(uv.x + d.x, uv.y      )).xyz;

    output += kernel * Color.Sample(Sampler, float2(uv.x - d.x, uv.y + d.y)).xyz;
    output += kernel * Color.Sample(Sampler, float2(uv.x      , uv.y + d.y)).xyz;
    output += kernel * Color.Sample(Sampler, float2(uv.x + d.x, uv.y + d.y)).xyz;

    return output;
}

[RootSignature(RS)]
PixelOutput main(VertexOutput input) {
    PixelOutput output;

    const float3 Tint = float3(TintR, TintG, TintB);

    const float3 blurred_color = BoxBlur3x3(uint2(DimensionsX, DimensionsY), input.Texture, Glasses);
    const float3 tinted_color = blurred_color * Tint;
    const float lum = Luminance(tinted_color);
    const float3 saturated_color = lerp(float3(lum, lum, lum), tinted_color, Saturation);
    const float3 tonemap_color = ACESFitted(saturated_color);

    output.Color = float4(tonemap_color, 1.0f);

    return output;
}