#include "CopyImage.hlsli"

cbuffer VignetteParams : register(b0)
{
    float vignetteScale; // 例: 16.0f
    float vignettePower; // 例: 0.8f
    float pad0;
    float pad1;
    float4 tint; // 外枠用色（RGBA）
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
    float4 base = gTexture.Sample(gSampler, input.texcoord);

    // ビネット係数（中心≒1、外枠≒0）
    float2 correct = input.texcoord * (1.0f - input.texcoord.yx);
    float vig = correct.x * correct.y * vignetteScale;
    vig = saturate(pow(vig, vignettePower));

    // まず従来の暗部（中心は明るく、外枠は暗く）を適用
    float3 darkened = base.rgb * vig;

    // 外枠マスク（中心≒0、外枠≒1）。赤枠はこのマスクのみで色付けする
    // 立ち上がりをやや急峻にして、枠がにじみ過ぎないようにする
    float edgeMask = pow(1.0f - vig, 2.0f); // 外側ほど強く

    // 外枠カラーを加算寄りで合成（中心には影響しない）
    float3 rim = tint.rgb * (edgeMask * tint.a);

    float3 result = darkened + rim;
    output.color = float4(result, base.a);
    return output;
}
