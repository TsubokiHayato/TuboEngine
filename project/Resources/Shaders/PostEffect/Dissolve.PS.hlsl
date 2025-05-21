#include "CopyImage.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

float rand2d1d(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898f, 78.233f))) * 43758.5453f);
}

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float dissolveThreshold = 0.5f; // 0.0～1.0で変化させるとアニメーション
    float noise = rand2d1d(input.texcoord);

    if (noise < dissolveThreshold)
    {
        output.color = gTexture.Sample(gSampler, input.texcoord);
    }
    else
    {
        output.color = float4(0, 0, 0, 1); // 黒で消す
    }

    return output;
}
