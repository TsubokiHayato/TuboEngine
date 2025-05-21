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

    // ブラーの中心（画面中央）
    const float2 kCenter = float2(0.5f, 0.5f);
    // サンプリング数
    const int kNumSamples = 10;
    // ブラーの強さ（大きいほど広がる）
    const float kBlurWidth = 0.02f;

    // 中心から現在のuvへの方向ベクトル
    float2 direction = input.texcoord - kCenter;

    float3 sum = float3(0.0f, 0.0f, 0.0f);

    // 放射状にサンプリング
    for (int i = 0; i < kNumSamples; ++i)
    {
        float t = float(i) / (kNumSamples - 1);
        float2 sampleUV = input.texcoord - direction * kBlurWidth * t;
        sum += gTexture.Sample(gSampler, sampleUV).rgb;
    }

    sum /= kNumSamples;
    output.color = float4(sum, 1.0f);
    return output;
}
