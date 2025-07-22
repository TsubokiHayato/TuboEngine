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
	/// <summary>
	/// シングルトンインスタンス取得
	/// </summary>
	static OffScreenRendering* GetInstance() {
		if (!instance) {
			instance = new OffScreenRendering();
		}
		return instance;
	}


private:
	// コンストラクタ・デストラクタ・コピー禁止
	static OffScreenRendering* instance;
	OffScreenRendering() = default;
	~OffScreenRendering() = default;
	OffScreenRendering(const OffScreenRendering&) = delete;
	OffScreenRendering& operator=(const OffScreenRendering&) = delete;

public:
	///------------------------------------------------------------------------
	///                             メンバ関数
	///------------------------------------------------------------------------

	/// <summary>
	/// 初期化処理
	/// 必要なリソースの生成や各種設定を行います。
	/// </summary>
	void Initialize();

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

	 void TransitionDepthTo(D3D12_RESOURCE_STATES newState);


	/// <summary>
	/// 描画処理
	/// レンダーテクスチャへの描画を行います。
	/// </summary>
	void Draw();

	void DrawImGui();


	void Finalize();

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

private:

	///-----------------------------------------------------------------------
	///                             メンバ変数
	///-----------------------------------------------------------------------

	// D3D12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;

	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;


	// オフスクリーン用RTVヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> offscreenRtvDescriptorHeap;

	// オフスクリーン用RTVハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE offscreenRtvHandle{};

	// レンダーターゲットのクリアカラー（赤色）
	const Vector4 kRenderTargetClearValue = {0.1f, 0.25f, 0.5f, 1.0f}; // RGBの値。青っぽい色;

	// レンダーテクスチャリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource_;

	// レンダーテクスチャの現在の状態
	D3D12_RESOURCE_STATES renderTextureState = D3D12_RESOURCE_STATE_RENDER_TARGET;

	// リソースバリア構造体
	D3D12_RESOURCE_BARRIER renderingBarrier{};
	D3D12_RESOURCE_BARRIER depthBarrier{};
	D3D12_RESOURCE_STATES depthResourceState_ = D3D12_RESOURCE_STATE_DEPTH_WRITE;
   

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
