#include "CopyImage.hlsli"
cbuffer DissolveParams : register(b0)
{
    float dissolveThreshold; // 0.0～1.0で制御
}

Texture2D<float4> gTexture : register(t0);
Texture2D<float> gMaskTexture : register(t1);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float mask = gMaskTexture.Sample(gSampler, input.texcoord);
    
    //maskが閾値以下ならdiscard
    if (mask <= 0.5f)
    {
        discard;
    }
    
    output.color = gTexture.Sample(gSampler, input.texcoord);

    return output;
}
