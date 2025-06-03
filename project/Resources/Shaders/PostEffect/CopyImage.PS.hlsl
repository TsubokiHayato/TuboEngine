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
    
    
    /*Normal*/
    output.color = gTexture.Sample(gSampler, input.texcoord);
    
    
    /*OutLine*/
    //ここは難しいのでいったんスキップ
    
    
    
    return output;
}
