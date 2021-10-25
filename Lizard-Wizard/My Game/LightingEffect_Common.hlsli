#ifndef LIGHTINGEFFECT_COMMON_HLSLI
#define LIGHTINGEFFECT_COMMON_HLSLI

#define LightingEffectRS \
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
"    visibility = SHADER_VISIBILITY_PIXEL)" \

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
    uint _pad0;
    uint _pad1;
    uint _pad2;
    float4 LightPos[254];
    float4 LightCol[254];
}

//cbuffer LightPositions : register(b1) {
//}
//
//cbuffer LightColors : register(b2) {
//}

struct PixelOutput {
    float4 Color : SV_TARGET0;
};

#endif