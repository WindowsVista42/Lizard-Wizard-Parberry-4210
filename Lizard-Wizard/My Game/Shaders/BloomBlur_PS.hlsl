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
}

struct PixelOutput {
    float4 Color : SV_TARGET0;
};

static const float Intensity = 0.04f;

float3 BoxBlur3x3(float2 uv, float intensity) {
    float2 d = (float2(1.0f, 1.0f) / 3.0) * intensity;
    const float kernel = 1.0f / 9.0f;

    float3 output;
    output  = kernel * Color.Sample(Sampler, float2(uv.x - 2.0 * d.x, uv.y - d.y)).xyz;
    output += kernel * Color.Sample(Sampler, float2(uv.x            , uv.y - d.y)).xyz;
    output += kernel * Color.Sample(Sampler, float2(uv.x + 2.0 * d.x, uv.y - d.y)).xyz;

    output += kernel * Color.Sample(Sampler, float2(uv.x - 2.0 * d.x, uv.y      )).xyz;
    output += kernel * Color.Sample(Sampler, float2(uv.x            , uv.y      )).xyz;
    output += kernel * Color.Sample(Sampler, float2(uv.x + 2.0 * d.x, uv.y      )).xyz;

    output += kernel * Color.Sample(Sampler, float2(uv.x - 2.0 * d.x, uv.y + d.y)).xyz;
    output += kernel * Color.Sample(Sampler, float2(uv.x            , uv.y + d.y)).xyz;
    output += kernel * Color.Sample(Sampler, float2(uv.x + 2.0 * d.x, uv.y + d.y)).xyz;

    return output;
}

[RootSignature(RS)]
PixelOutput main(VertexOutput input) {
    PixelOutput output;

    float3 color = BoxBlur3x3(input.Texture, Intensity);

    output.Color = float4(color, 1.0f);

    return output;
}
