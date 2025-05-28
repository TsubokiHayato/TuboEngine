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
    
    
    /*OutLine*/
    //ここは難しいのでいったんスキップ
    
    
    /*Dissolve*/
    
    
    
    /*randam(未完成)*/
    //動かすにはCPUから値を受け取らなきゃいけない
    float randam = rand2d1d(input.texcoord);
    
    //色にする
    output.color = float4(randam, randam, randam, 1.0f);
    
    
    
    return output;
}
