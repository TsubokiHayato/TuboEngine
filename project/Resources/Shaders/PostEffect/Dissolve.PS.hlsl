cbuffer DissolveParams : register(b0)
{
    float dissolveThreshold; // 0.0～1.0で制御
}

Texture2D<float4> gTexture : register(t0);
Texture2D<float> gMaskTexture : register(t1);
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

    float mask = gMaskTexture.Sample(gSampler, input.texcoord);

    // ノイズを加えてDissolve境界をランダムに
    float noise = rand2d1d(input.texcoord * 100.0);
    float dissolve = mask + (noise - 0.5) * 0.1;

    // Dissolveしきい値でピクセルを破棄
    if (dissolve < dissolveThreshold)
    {
        discard;
    }

    // 境界付近にグローを追加（例：オレンジ色）
    float glow = smoothstep(dissolveThreshold, dissolveThreshold + 0.03, dissolve);
    float4 baseColor = gTexture.Sample(gSampler, input.texcoord);
    float3 glowColor = lerp(baseColor.rgb, float3(1.0, 0.5, 0.0), glow * 0.8);

    output.color = float4(glowColor, baseColor.a);

    return output;
}
