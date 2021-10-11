#include "GameEffect_Common.hlsli"

[RootSignature(GameEffectRS)]
VSOutput main( float4 position : SV_POSITION ) {
	VSOutput vout;

	vout.PositionPS = mul(position, WorldViewProjection);

	return vout;
}