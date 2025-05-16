#include "CopyImage.hlsli"

//パイ
static const float PI = 3.14159265358979323846f;

//インデックスマトリックスの
static const float2 kIndex3x3[3][3] =
{
    { { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f } },
    { { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { -1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } },
};

//カーネルマトリックス
static const float kKernel3x3[3][3] =
{
    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f },
    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f },
    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f },
};

float gauss(float x, float y, float sigma)
{
    float exponent = -(x * x + y * y) / rcp(2.0f * sigma * sigma);
    float denominator = 2.0f * PI * sigma * sigma;
    return exp(exponent) * rcp(denominator);

}

float rand2d1d(float2 uv)
{
       // 2D座標を1Dのランダム値に変換する関数  
       // ハッシュ関数を使用して擬似乱数を生成  
    return frac(sin(dot(uv, float2(12.9898f, 78.233f))) * 43758.5453f);
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
    
    
    /*Normal*/
    //output.color = gTexture.Sample(gSampler, input.texcoord);
    
    /*Grayscale*/
    //output.color = gTexture.Sample(gSampler, input.texcoord);
    //float value = dot(output.color.rgb, float3(0.2125f, 0.7154, 0.0721f));
    //output.color.rgb = float3(value, value, value);
    
    /*Vignette*/
    //output.color = gTexture.Sample(gSampler, input.texcoord);
    ////周辺を0に、中心になるほど明るくなるように計算で調整
    //float2 correct = input.texcoord * (1.0f - input.texcoord.yx);
    ////corect だけで計算すると中心の最大値が0.0625で暗すぎるのでScaleで調整。
    //float vignette = correct.x * correct.y * 16.0f;
    ////とりあえず0.8乗でそれっぽくしてみた
    //vignette = saturate(pow(vignette, 0.8f));
    ////係数として乗算
    //output.color.rgb *= vignette;
    
    ////16.0fや8.0fは調整できるようにする
    
    /*Smoothing*/
    //uint width, height; // 1. uvStepSizeの算出
    //gTexture.GetDimensions(width, height);
    //float2 uvStepSize = float2(rcp(width), rcp(height));


    //output.color.rgb = float3(0.0f, 0.0f, 0.0f);
    //output.color.a = 1.0f;

    //for (int x = 0; x < 3; ++x)
    //{ // 2. 3×3ループ
    //    for (int y = 0; y < 3; ++y)
    //    {
    //// 3. 現在のtexcoordを算出
    //        float2 texcoord = input.texcoord + kIndex3x3[x][y] * uvStepSize;
    
    //// 4. 色に1/9掛けて足す
    //        float3 fetchColor = gTexture.Sample(gSampler, texcoord).rgb;
    //        output.color.rgb += fetchColor * kKernel3x3[x][y];
    //    }
    //}
    
    //    /*GaussianFilter*/
    //    //この処理は重いので要注意
    //    //あまり変化が見られないがカーネルサイズを上げると効果が見られる
    //    float weight = 0.0f;
    //    float kernel3x3[3][3];

    //    uint width, height;
    //    gTexture.GetDimensions(width, height);
    //    float2 uvStepSize = float2(rcp(width), rcp(height));

    //    output.color.rgb = float3(0.0f, 0.0f, 0.0f);
    //    output.color.a = 1.0f;

    //    for (int x = 0; x < 3; ++x)
    //    {
    //        for (int y = 0; y < 3; ++y)
    //        {
    //        // ここの2.0fはσの値。外から渡せるようにしたい
    //            kernel3x3[x][y] = gauss(kIndex3x3[x][y].x, kIndex3x3[x][y].y, 2.0f);
    //            weight += kernel3x3[x][y];

    //            float2 texcoord = input.texcoord + kIndex3x3[x][y] * uvStepSize;
    //            float3 fetchColor = gTexture.Sample(gSampler, texcoord).rgb;
    //            output.color.rgb += fetchColor * kernel3x3[x][y];
    //        }
        //    }

    //// 正規化: 合計した色を重みの合計で割る
    //    output.color.rgb /= weight;
    //    output.color.a = 1.0f;

    
    
    /*OutLine*/
    //ここは難しいのでいったんスキップ
    

    
    ///*Radial Blur*/
    //const float2 kCenter = float2(0.5f, 0.5f); // 中心点 ここを基準に放射状にブラーがかかる
    //const int kNumSamples = 10; // サンプリング数。多いほど滑らかだが重い
    //const float kBlurWidth = 0.02f; // ぼかしの幅。大きいほど大きい

    //// 中心から現在のuvに対しての方向を計算。
    
    //float2 direction = input.texcoord - kCenter;
    //// 単位方向とはいえ、単位ベクトルだが、ここではあえて正規化せず、遠いほどより遠くをサンプリングする

    //float3 outputColor = float3(0.0f, 0.0f, 0.0f);
    //for (int sampleIndex = 0; sampleIndex < kNumSamples; ++sampleIndex)
    //{
    //// 現在のカラーをぼかすほど計算した方向にサンプリング点を進めながらサンプリングしていく
    //    float2 texcoord = input.texcoord + direction * kBlurWidth * float(sampleIndex);
    //    outputColor.rgb += gTexture.Sample(gSampler, texcoord).rgb;
    //}

    //// 平均化する
    //outputColor.rgb *= rcp(kNumSamples);

    
    //output.color.rgb = outputColor;
    //output.color.a = 1.0f;
    
    /*Dissolve*/
    
    
    
    /*randam(未完成)*/
    //動かすにはCPUから値を受け取らなきゃいけない
    float randam = rand2d1d(input.texcoord);
    
    //色にする
    output.color = float4(randam, randam, randam, 1.0f);
    
    
    
    return output;
}
