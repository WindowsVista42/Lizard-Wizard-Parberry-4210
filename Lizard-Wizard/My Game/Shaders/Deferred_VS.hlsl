#include "Deferred_Common.hlsli"

[RootSignature(DeferredEffectRS)]
VertexOutput main(VertexInput input) {
	VertexOutput output;

	output.VertexPosition = mul(float4(input.Position, 1.0), WorldViewProjection);

	output.Normal = normalize(mul(float4(input.Normal, 1.0f), TransposeWorld).xyz);
	output.Position = mul(float4(input.Position, 1.0), World);
	output.Texture = input.Texture;

	return output;
}