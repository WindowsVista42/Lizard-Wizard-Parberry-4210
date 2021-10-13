#include "GameEffect_Common.hlsli"

[RootSignature(GameEffectRS)]
PSOutput main(VSOutput pin) {
	PSOutput pout;

	pout.Diffuse = float4((pin.NormalPS + 1.0f) * 0.5f, 1.0f);

	return pout;
}