#include "PostProcess_Common.hlsli"

VertexOutput main(uint vertex_index : SV_VertexId) {
    VertexOutput output;

    // "Big Triangle" optimization so that we only draw 3 vertices instead of 4
    output.Texture = float2((vertex_index << 1) & 2, vertex_index & 2);
    output.VertexPosition = float4(output.Texture.x * 2.0f - 1.0f, -output.Texture.y * 2.0f + 1.0f, 0.0f, 1.0f);

    return output;
}