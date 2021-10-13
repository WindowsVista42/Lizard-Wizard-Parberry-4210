#include "GameEffect_Common.hlsli"

[RootSignature(GameEffectRS)]
VSOutput main(VSInput vin) {
	VSOutput vout;

	vout.PositionPS = mul(float4(vin.PositionVS, 1.0), WorldViewProjection);
	vout.NormalPS = vin.NormalVS;

	return vout;
}