#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <memory>
#include "DirectXCommon.h"
#include "PostEffectManager.h"

// 前方宣言
class WinApp;
class DirectXCommon;
class OffScreenRenderingPSO;
class GrayScalePSO;
class VignettePSO;

// オフスクリーンレンダリングを管理するクラス
// レンダーテクスチャへの描画やリソースバリアの制御などを担当します。
class OffScreenRendering
{
public:
	///------------------------------------------------------------------------
	///                             メンバ関数
	///------------------------------------------------------------------------

	/// <summary>
	/// 初期化処理
	/// 必要なリソースの生成や各種設定を行います。
	/// </summary>
	void Initialize(WinApp* winApp, DirectXCommon* dxCommon);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画前設定
	/// レンダーターゲットのセットやクリアなどを行います。
	/// </summary>
	void PreDraw();

	/// <summary>
	/// レンダーテクスチャをシェーダリソース用にバリア遷移します。
	/// </summary>
	void TransitionRenderTextureToShaderResource();

	/// <summary>
	/// レンダーテクスチャをレンダーターゲット用にバリア遷移します。
	/// </summary>
	void TransitionRenderTextureToRenderTarget();

	///<summary>
	/// レンダーテクスチャを深度ステンシル用にバリア遷移します。
	///</summary>
	void TransitionRenderTextureToDepthStencil();

	///<summary>
	/// レンダーテクスチャをオフスクリーン用にバリア遷移します。
	///</summary>
	void TransitionRenderTextureToOffScreen();

	/// <summary>
	/// 描画処理
	/// レンダーテクスチャへの描画を行います。
	/// </summary>
	void Draw();

	void DrawImGui();

	/// <summary>
	/// レンダーターゲットリソースの作成
	/// 指定されたサイズ・フォーマット・クリアカラーでリソースを生成します。
	/// </summary>
	/// <param name="device">D3D12デバイス</param>
	/// <param name="width">幅</param>
	/// <param name="height">高さ</param>
	/// <param name="format">フォーマット</param>
	/// <param name="clearColor">クリアカラー</param>
	/// <returns>作成されたリソース</returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTargetResource(
		Microsoft::WRL::ComPtr<ID3D12Device>& device,
		int32_t width,
		int32_t height,
		DXGI_FORMAT format,
		const Vector4& clearColor
	);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilResource(
		Microsoft::WRL::ComPtr<ID3D12Device>& device,
		int32_t width,
		int32_t height,
		DXGI_FORMAT format,
		const Vector4& clearColor
	);

    public:  
    Matrix4x4 SetViewProjection(const Matrix4x4& viewProjection) {  
       postEffectManager.SetOutlineProjection(viewProjection);  
       return viewProjection;  
    }

private:
	///-----------------------------------------------------------------------
	///                             受取り用変数
	///-----------------------------------------------------------------------
	// DirectX共通部分へのポインタ
	DirectXCommon* dxCommon_ = nullptr;

	// ウィンドウアプリケーションへのポインタ
	WinApp* winApp_ = nullptr;

	///-----------------------------------------------------------------------
	///                             メンバ変数
	///-----------------------------------------------------------------------

	// D3D12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;

	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;


	// オフスクリーン用RTVヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> offscreenRtvDescriptorHeap;
	//深度用のRTVヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> offscreenDepthRtvDescriptorHeap;

	// オフスクリーン用RTVハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE offscreenRtvHandle{};
	// 深度用RTVハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE offscreenDepthRtvHandle{};

	// レンダーターゲットのクリアカラー（赤色）
	const Vector4 kRenderTargetClearValue = { 1.0f, 0.0f, 0.0f, 1.0f };
	// オフスクリーンレンダリング用のクリアカラー（白色）
	const Vector4 kOffScreenClearValue = { 1.0f, 1.0f, 1.0f, 1.0f };

	// レンダーテクスチャリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource_;
	// 深度テクスチャリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> depthTextureResource_;

	// レンダーテクスチャの現在の状態
	D3D12_RESOURCE_STATES renderTextureState_ = D3D12_RESOURCE_STATE_RENDER_TARGET;


	// リソースバリア構造体
	D3D12_RESOURCE_BARRIER renderingBarrier{};
	//　深度バリア
	D3D12_RESOURCE_BARRIER depthRenderingBarrier{};

	///--------------------------------------------------------------------------
	///                             PSO関連
	///---------------------------------------------------------------------------

	// オフスクリーン用PSOクラス
	OffScreenRenderingPSO* offScreenRenderingPSO = nullptr;

	// ヴィネット用PSOクラス
	VignettePSO* vignettePSO = nullptr;
	///-----------------------------------------------------------------------
	///                             リソース
	///------------------------------------------------------------------------

	Microsoft::WRL::ComPtr <ID3D12Resource> vignetteResource;
	//VignetteParams* vignetteData = nullptr;

	///-----------------------------------------------------------------------
	///                             PostEffectManager
	///-----------------------------------------------------------------------

	PostEffectManager postEffectManager;
};
