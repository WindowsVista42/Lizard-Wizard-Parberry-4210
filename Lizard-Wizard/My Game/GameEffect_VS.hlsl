#include "GameEffect_Common.hlsli"

[RootSignature(GameEffectRS)]
VertexOutput main(VertexInput input) {
	VertexOutput output;

	output.VertexPosition = mul(float4(input.Position, 1.0), WorldViewProjection);

	output.Normal = input.Normal;
	output.Position = mul(float4(input.Position, 1.0), World);
	output.Texture = input.Texture;

	return output;
}