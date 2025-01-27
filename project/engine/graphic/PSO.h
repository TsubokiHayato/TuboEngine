#pragma once
#include"DirectXCommon.h"
#include"BlendMode.h"
class PSO
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectX共通部分</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 共通描画設定
	/// </summary>
	void DrawSettingsCommon();

public:
    /// <summary>
    /// ブレンドモードの設定
    /// </summary>
    /// <param name="blendMode">ブレンドモード</param>
    /// <remarks>
    /// 0: None
    /// 1: Normal
    /// 2: Add
    /// 3: Subtract
    /// 4: Multily
    /// 5: Screen
    /// </remarks>
    void SetBlendMode(int blendMode) {
    switch (blendMode)
    {
    case 0:
    this->blendMode = kBlendModeNone;
    break;
    case 1:
    this->blendMode = kBlendModeNormal;
    break;
    case 2:
    this->blendMode = kBlendModeAdd;
    break;
    case 3:
    this->blendMode = kBlendModeSubtract;
    break;
    case 4:
    this->blendMode = kBlendModeMultily;
    break;
    case 5:
    this->blendMode = kBlendModeScreen;
    break;
    default:
    this->blendMode = kBlendModeNone;
    break;
    }
    }


		
		
		
		

	/// <summary>
	/// ブレンドモードの取得
	/// </summary>
	/// <returns>ブレンドモード</returns>
	BlendMode GetBlendMode()const { return blendMode; }

private:
	/*---------------------------------------------------
			関数
	---------------------------------------------------*/

	/// <summary>
	/// ルートシグネイチャの作成
	/// </summary>
	void CreateRootSignature();

	/// <summary>
	/// グラフィックスパイプラインの作成
	/// </summary>
	void CreateGraphicPipeline();

	/*-----------------
		rootSignature
	-----------------*/
	DirectXCommon* dxCommon_;

	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	//DescriptorRange作成
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	//RootParameter作成。
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	//Sampler作成
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	//シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr <ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr <ID3DBlob> errorBlob = nullptr;
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;
	/*------------
	  InputLayOut
	------------*/
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	/*------------
	  BlendState
	------------*/

	D3D12_BLEND_DESC blendDesc{};
	/*------------------
	  RasterizerState
	------------------*/

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	/*-------------------
	  Vertex&Pixel_Shader
	-------------------*/

	//Shaderをコンパイルする
	Microsoft::WRL::ComPtr <IDxcBlob> vertexShaderBlob;

	Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlob;
	/*---------------
	DepthStencilDescの設定
	-------------------*/

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	/*------------------
	 　 PSOを生成する
	------------------*/
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicPipelineStateDesc{};
	Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipeLineState = nullptr;

	Microsoft::WRL::ComPtr <ID3D12Device> device;
	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList;


	BlendMode blendMode = kBlendModeNormal;

};

