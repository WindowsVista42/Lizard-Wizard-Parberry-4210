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

float Luminance(float3 color) {
    return dot(color, float3(0.299f, 0.587f, 0.114f));
}

static const float Threshold = 2.0f;

[RootSignature(RS)]
PixelOutput main(VertexOutput input) {
    PixelOutput output;

    float3 color = Color.Sample(Sampler, input.Texture);
    float lum = Luminance(color);

    float F = clamp(lum - Threshold, 0.0f, Threshold) / Threshold;

    output.Color = float4(F * color, 1.0f);

    return output;
}
