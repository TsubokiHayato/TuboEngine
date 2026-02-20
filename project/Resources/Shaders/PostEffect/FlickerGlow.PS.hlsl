#include "CopyImage.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer FlickerGlowParams : register(b0)
{
    float time;           // 経過時間
    float intensity;      // 全体の強さ
    float noiseAmount;    // ノイズ量
    float glowStrength;   // グローの乗算
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

// 2D ノイズ
float rand2d(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898f,78.233f))) * 43758.5453f);
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float4 src = gTexture.Sample(gSampler, input.texcoord);

    // 徐々に点灯 (0→1 のイージング)
    float t = saturate(time * intensity);
    float lightFactor = smoothstep(0.0f, 1.0f, t);

    // チラつきノイズ
    float2 noiseUV = input.texcoord * 400.0f + float2(time * 37.0f, time * 17.0f);
    float nR = rand2d(noiseUV);
    float nG = rand2d(noiseUV + 10.0f);
    float nB = rand2d(noiseUV + 20.0f);
    float3 noiseRGB = (float3(nR, nG, nB) - 0.5f) * 2.0f * noiseAmount;

    // ベース色 + ノイズ
    float3 base = src.rgb * lightFactor + noiseRGB;

    // 軽いグロー : 自ピクセルと周囲サンプルの平均
    int w, h;
    gTexture.GetDimensions(w, h);
    float2 texel = 1.0f / float2((float)w, (float)h);
    float3 sum = base;
    sum += gTexture.Sample(gSampler, input.texcoord + float2( texel.x,  0.0f)).rgb;
    sum += gTexture.Sample(gSampler, input.texcoord + float2(-texel.x,  0.0f)).rgb;
    sum += gTexture.Sample(gSampler, input.texcoord + float2( 0.0f,  texel.y)).rgb;
    sum += gTexture.Sample(gSampler, input.texcoord + float2( 0.0f, -texel.y)).rgb;
    float3 glow = sum / 5.0f;

    float3 finalColor = lerp(base, glow, glowStrength);

    output.color = float4(finalColor, src.a);
    return output;
}
