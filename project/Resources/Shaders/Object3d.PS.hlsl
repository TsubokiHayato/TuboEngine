#include"Object3d.hlsli"

struct Material
{
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
    float shininess;
};


struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};

struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
};

struct Camera
{
    float3 worldPosition;
};

struct LightType
{
    //0 : 平行光源
	//1 : Phong反射モデル
	//2 : Blinn-Phong反射モデル
    int type;
};


//コンスタントバッファの定義
//使用例 : ConstantBuffer<構造体> 変数名 : register(b0);
ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

ConstantBuffer<Camera> gCamera : register(b2);
ConstantBuffer<LightType> gLightType : register(b3);
ConstantBuffer<PointLight> gPointLight : register(b4);

struct PixcelShaderOutput
{
    float4 color : SV_TARGET0;
};


PixcelShaderOutput main(VertexShaderOutPut input)
{
   
    PixcelShaderOutput output;
    float4 tranceformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, tranceformedUV.xy);
   
  
    if (gMaterial.enableLighting != 0)
    {
        //平行光源
        if (gLightType.type == 0)
        {
            float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
            float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
            output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
            output.color.a = gMaterial.color.a * textureColor.a;
        }
        //PhongReflection
        else if (gLightType.type == 1)
        {
            float NdotL = dot(normalize(input.normal), normalize(-gDirectionalLight.direction));
            float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
            float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
            float3 reflectLight = reflect(normalize(gDirectionalLight.direction), normalize(input.normal));
        
            float RtoE = dot(reflectLight, toEye);
            float specularPow = pow(saturate(RtoE), gMaterial.shininess);
        
            float3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        
            float3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);
        
            output.color.rgb = diffuse + specular;
            output.color.a = gMaterial.color.a * textureColor.a;
        }
        //BlinnPhong
        else if (gLightType.type == 2)
        {
            
            float NdotL = dot(normalize(input.normal), normalize(-gDirectionalLight.direction));
            float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
            float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
            float3 reflectLight = reflect(normalize(gDirectionalLight.direction), normalize(input.normal));

            float3 halfVector = normalize(-gDirectionalLight.direction + toEye);
            float NDotH = dot(normalize(input.normal), halfVector);
            float specularPow = pow(saturate(NDotH), gMaterial.shininess);

            float3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
            float3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);

            output.color.rgb = diffuse + specular;
            output.color.a = gMaterial.color.a * textureColor.a;
      
        }
        //PointLight
        else if (gLightType.type == 3)
        {
    // Directional Light
            float NdotL = dot(normalize(input.normal), normalize(-gDirectionalLight.direction));
            float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
            float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
            float3 reflectLight = reflect(normalize(gDirectionalLight.direction), normalize(input.normal));
            float RtoE = dot(reflectLight, toEye);
            float specularPow = pow(saturate(RtoE), gMaterial.shininess);

            float3 diffuseDirectionalLight = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
            float3 specularDirectionalLight = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);

    // Point Light
            float3 pointLightDirection = normalize(gPointLight.position - input.worldPosition);
            float NdotLPoint = dot(normalize(input.normal), pointLightDirection);
            float cosPoint = pow(NdotLPoint * 0.5f + 0.5f, 2.0f);
            float3 reflectPointLight = reflect(-pointLightDirection, normalize(input.normal));
            float RtoEPoint = dot(reflectPointLight, toEye);
            float specularPowPoint = pow(saturate(RtoEPoint), gMaterial.shininess);

            float3 diffusePointLight = gMaterial.color.rgb * textureColor.rgb * gPointLight.color.rgb * cosPoint * gPointLight.intensity;
            float3 specularPointLight = gPointLight.color.rgb * gPointLight.intensity * specularPowPoint * float3(1.0f, 1.0f, 1.0f);

    // Combine lights
            output.color.rgb = diffuseDirectionalLight + specularDirectionalLight + diffusePointLight + specularPointLight;
            output.color.a = gMaterial.color.a * textureColor.a;
        }
        
        
    }
    else
    {
        output.color = gMaterial.color * textureColor;
    }
    
    
    if (textureColor.a <= 0.5f)
    {
        discard;
    }
    if (textureColor.a == 0.0f)
    {
        discard;
    }
    if (output.color.a == 0.0f)
    {
        discard;
    }
  
    
    return output;
};




