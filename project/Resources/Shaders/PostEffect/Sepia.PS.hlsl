#include "CopyImage.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 color = gTexture.Sample(gSampler, input.texcoord);

    float r = dot(color.rgb, float3(0.393f, 0.769f, 0.189f));
    float g = dot(color.rgb, float3(0.349f, 0.686f, 0.168f));
    float b = dot(color.rgb, float3(0.272f, 0.534f, 0.131f));

    output.color = float4(r, g, b, color.a);
    return output;
}
