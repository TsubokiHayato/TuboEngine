#include "CopyImage.hlsli"

Texture2D<float4> gTexture : register(t0);
Texture2D<float> gDepthTexture : register(t1);
SamplerState gSampler : register(s0);
SamplerState gSamplerPoint : register(s1);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

cbuffer Material : register(b0)
{
    float4x4 projectionInverse;
};

// Prewittフィルタ用カーネル（横方向・縦方向）
static const float kPrewittHorizontalKernel[3][3] =
{
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
};

static const float kPrewittVerticalKernel[3][3] =
{
    { -1.0f / 6.0f, -1.0f / 6.0f, -1.0f / 6.0f },
    { 0.0f, 0.0f, 0.0f },
    { 1.0f / 6.0f, 1.0f / 6.0f, 1.0f / 6.0f },
};


static const float2 kIndex3x3[3][3] =
{
    { { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f } },
    { { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { -1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } },
};


// RGBを輝度に変換する関数
float Luminance(float3 v)
{
    return dot(v, float3(0.2125f, 0.7154f, 0.0721f));
}



PixelShaderOutput main(VertexShaderOutput input)
{
    // // 畳み込み処理
    //float2 difference = float2(0.0f, 0.0f);
    //uint width, height;
    //gTexture.GetDimensions(width, height);
    //float2 uvStepSize = float2(1.0f / width, 1.0f / height);
    //for (int x = 0; x < 3; ++x)
    //{
    //    for (int y = 0; y < 3; ++y)
    //    {
    //        float2 texcoord = input.texcoord + kIndex3x3[x][y] * uvStepSize;
            
    //        float3 fetchColor = gTexture.Sample(gSampler, texcoord).rgb;
    //        float luminance = Luminance(fetchColor);
    //        difference.x += luminance * kPrewittHorizontalKernel[x][y];
    //        difference.y += luminance * kPrewittVerticalKernel[x][y];
    //    }
    //}
    // // エッジ強度の計算と出力
    //float weight = length(difference);
    //weight = saturate(weight*6.0f);
    //PixelShaderOutput output;
    //output.color.rgb = (1.0f - weight) * gTexture.Sample(gSampler, input.texcoord).rgb;
    //output.color.a = 1.0f;
    
    /*OutLine*/
    
    // 畳み込み処理
    float2 difference = float2(0.0f, 0.0f);
    
    uint width, height;
    gTexture.GetDimensions(width, height);
    float2 uvStepSize = float2(1.0f / width, 1.0f / height);
    
    for (int x = 0; x < 3; ++x)
    {
        for (int y = 0; y < 3; ++y)
        {
            float2 texcoord = input.texcoord + kIndex3x3[x][y] * uvStepSize;
            
            float ndcDepth = gDepthTexture.Sample(gSamplerPoint, texcoord);
            //DSC->View。P^{-1}においてxとyはzwに影響を与えないので何でもいい。なので、わざわざ行列を渡さなくてもいい
            //gMaterial.projectionInverseはCBufferを使って渡しておくこと
            float4 viewSpace = mul(float4(0.0f, 0.0f, ndcDepth, 1.0f), projectionInverse);
            float viewZ = viewSpace.z / rcp(viewSpace.w); //同次座標系からデカルト座標系へ変換
            difference.x += viewZ * kPrewittHorizontalKernel[x][y];
            difference.y += viewZ * kPrewittVerticalKernel[x][y];
        }
    }

    // エッジ強度の計算と出力
    float weight = length(difference);
    weight = saturate(weight);
    PixelShaderOutput output;
   
    output.color.rgb = (1.0f - weight) * gTexture.Sample(gSampler, input.texcoord).rgb;
    output.color.a = 1.0f;


    
    
    return output;
}
