#ifndef GAMEEFFECT_COMMON_HLSLI
#define GAMEEFFECT_COMMON_HLSLI

#define GameEffectRS \
"RootFlags ("\
"    ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |"\
"    DENY_DOMAIN_SHADER_ROOT_ACCESS |"\
"    DENY_GEOMETRY_SHADER_ROOT_ACCESS |"\
"    DENY_HULL_SHADER_ROOT_ACCESS ),"\
"CBV(b0)"

cbuffer GameEffectConstants : register(b0) {
	float4x4 WorldViewProjection;
}

struct VSInput {
	float3 PositionVS : SV_POSITION;
	float3 NormalVS : NORMAL;
	float2 TextureVS : TEXCOORD;
};

struct VSOutput {
	float4 PositionPS : SV_Position;
	float3 NormalPS : NORMAL0;
};

struct PSOutput {
	float4 Diffuse : SV_TARGET0;
};

#endif