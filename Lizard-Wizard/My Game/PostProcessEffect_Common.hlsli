#ifndef GAMEEFFECT_COMMON_HLSLI
#define GAMEEFFECT_COMMON_HLSLI

#define PostProcessEffectRS \
"RootFlags ("\
"    ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |"\
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

Texture2D<float4> Diffuse : register(t0);
Texture2D<float4> Normal : register(t1);
Texture2D<float4> Position : register(t2);

sampler Sampler : register(s0);

cbuffer GameEffectConstants : register(b0) {}

struct VertexOutput {
    float4 VertexPosition : SV_Position;
    float2 Texture : TEXCOORD0;
};

struct PixelOutput {
    float4 BackBuffer : SV_TARGET0;
};

struct Light {
    float3 Position;
    float3 Color;
};

#endif