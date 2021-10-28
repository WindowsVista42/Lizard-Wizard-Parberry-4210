#include "PostProcess_Common.hlsli"

#define RS \
"RootFlags ("\
"    DENY_DOMAIN_SHADER_ROOT_ACCESS |"\
"    DENY_GEOMETRY_SHADER_ROOT_ACCESS |"\
"    DENY_HULL_SHADER_ROOT_ACCESS ),"\
"DescriptorTable(SRV(t0), visibility = SHADER_VISIBILITY_PIXEL)," \
"DescriptorTable(SRV(t1), visibility = SHADER_VISIBILITY_PIXEL)," \
"CBV(b0, visibility = SHADER_VISIBILITY_PIXEL)," \
"StaticSampler(s0," \
"    filter = FILTER_MIN_MAG_MIP_LINEAR," \
"    addressU = TEXTURE_ADDRESS_CLAMP," \
"    addressV = TEXTURE_ADDRESS_CLAMP," \
"    addressW = TEXTURE_ADDRESS_CLAMP," \
"    visibility = SHADER_VISIBILITY_PIXEL)"

Texture2D<float4> Color0 : register(t0);
Texture2D<float4> Color1 : register(t1);

sampler Sampler : register(s0);

cbuffer Constants : register(b0) {
    float scale;
}

struct PixelOutput {
    float4 Color : SV_TARGET0;
};

//static const float Intensity = 0.002f;

[RootSignature(RS)]
PixelOutput main(VertexOutput input) {
    PixelOutput output;

    float3 color0 = Color0.Sample(Sampler, input.Texture);
    float3 color1 = Color1.Sample(Sampler, input.Texture);

    output.Color = float4(color0 + color1, 1.0f);

    return output;
}
