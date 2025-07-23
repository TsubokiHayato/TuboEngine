#include "CopyImage.hlsli"

cbuffer ToonParams : register(b0)
{
    int stepCount; // 段階数
    float3 padding; // 16バイトアライメント用
    float4x4 projectionInverse; // プロジェクション逆行列
}

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    // テクスチャカラー取得
    float4 color = gTexture.Sample(gSampler, input.texcoord);

    // 明度（輝度）を計算
    float brightness = dot(color.rgb, float3(0.2125f, 0.7154f, 0.0721f));

    // 段階化
    float stepBrightness = floor(brightness * stepCount) / max(1, (stepCount - 1));
    float3 toonColor = color.rgb * stepBrightness;

    output.color = float4(toonColor, color.a);
    return output;
}