#ifndef TONEMAPEFFECT_COMMON_HLSLI
#define TONEMAPEFFECT_COMMON_HLSLI

#define TonemapEffectRS \
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
"    visibility = SHADER_VISIBILITY_PIXEL)" \

Texture2D<float4> Color : register(t0);

sampler Sampler : register(s0);

cbuffer GameEffectConstants : register(b0) {}

struct PixelOutput {
    float4 Color : SV_TARGET0;
};

#endif