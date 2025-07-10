#include "SkyBox.hlsli"

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
};

//頂点シェーダーへの入力頂点構造
struct VertexShaderInput
{
    float4 position : POSITION0;
    float3 texcoord : TEXCOORD0;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

VertexShaderOutPut main(VertexShaderInput input)
{
    VertexShaderOutPut output;
    output.position = mul(input.position, gTransformationMatrix.WVP).xywz;
    output.texcoord = input.position.xyz;
    return output;
}