#ifndef GAMEEFFECT_COMMON_HLSLI
#define GAMEEFFECT_COMMON_HLSLI

#define GameEffectRS \
"RootFlags ( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |" \
"            DENY_DOMAIN_SHADER_ROOT_ACCESS |" \
"            DENY_GEOMETRY_SHADER_ROOT_ACCESS |" \
"            DENY_HULL_SHADER_ROOT_ACCESS)," \
"DescriptorTable ( SRV(t0) )," \
"CBV(b0)"

cbuffer Constants : register(b0) {
	float4x4 WorldViewProjection;
}

struct VSOutput {
	float4 PositionPS : SV_Position;
};

#endif