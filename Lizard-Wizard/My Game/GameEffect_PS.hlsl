#include "GameEffect_Common.hlsli"

[RootSignature(GameEffectRS)]
float4 main() : SV_TARGET0 {
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}