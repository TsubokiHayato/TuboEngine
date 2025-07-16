#pragma once
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl/client.h>

///----------------------------------------------------
/// Line描画用のパイプラインステートオブジェクト管理クラス
///----------------------------------------------------
class LinePSO {

	///----------------------------------------------------
	///						メンバ関数
	///-----------------------------------------------------

public:
	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize();

	/// <summary>
	/// 描画設定を行う
	/// </summary>
	void DrawSettingsCommon();


private:
	/// <summary>
	/// ルートシグネイチャの作成
	/// </summary>
	void CreateRootSignature();

	/// <summary>
	/// グラフィックパイプラインステートオブジェクトの作成
	/// </summary>
	void CreateGraphicPipeline();


	///-----------------------------------------------------
	///						メンバ変数
	///-----------------------------------------------------

	private:

	// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	D3D12_ROOT_PARAMETER rootParameters[1] = {};
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};

	// BlendState
	D3D12_BLEND_DESC blendDesc{};

	// RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc{};

	// Shader
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob;

	// DepthStencilDesc
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

	// PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicPipelineStateDesc{};
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipeLineState = nullptr;


	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
};