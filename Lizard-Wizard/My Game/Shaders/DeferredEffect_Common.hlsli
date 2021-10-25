#ifndef DEFERREDEFFECT_COMMON_HLSLI
#define DEFERREDEFFECT_COMMON_HLSLI

#define DeferredEffectRS \
"RootFlags ("\
"    ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |"\
"    DENY_DOMAIN_SHADER_ROOT_ACCESS |"\
"    DENY_GEOMETRY_SHADER_ROOT_ACCESS |"\
"    DENY_HULL_SHADER_ROOT_ACCESS ),"\
"DescriptorTable(SRV(t0), visibility = SHADER_VISIBILITY_PIXEL)," \
"CBV(b0)," \
"StaticSampler(s0," \
"    filter = FILTER_MIN_MAG_MIP_LINEAR," \
"    addressU = TEXTURE_ADDRESS_CLAMP," \
"    addressV = TEXTURE_ADDRESS_CLAMP," \
"    addressW = TEXTURE_ADDRESS_CLAMP," \
"    visibility = SHADER_VISIBILITY_PIXEL)" \

Texture2D<float4> Color : register(t0);

sampler Sampler : register(s0);

cbuffer Constants : register(b0) {
    float4x4 WorldViewProjection;
    float4x4 World;
}

struct VertexInput {
    float3 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float2 Texture : TEXCOORD;
};

struct VertexOutput {
    float4 VertexPosition : SV_Position;
    float4 Position : Position0;
    float3 Normal : NORMAL0;
    float2 Texture : TEXCOORD0;
};

struct PixelOutput {
    float4 Diffuse : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 Position : SV_TARGET2;
};

struct Light {
    float3 Position;
    float3 Color;
};

#endif