#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <memory>
#include "DirectXCommon.h"

//前方宣言
class WinApp;
class DirectXCommon;
class OffScreenRenderingPSO;

class OffScreenRendering
{
public:

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectX共通部分</param>
	void Initialize(WinApp* winApp,DirectXCommon* dxCommon);
	/// <summary>
	/// 描画設定
	/// </summary>
	void PreDraw();

	void TransitionRenderTextureToShaderResource();
	void TransitionRenderTextureToRenderTarget();



	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTargetResource(Microsoft::WRL::ComPtr<ID3D12Device>& device, int32_t width, int32_t height, DXGI_FORMAT format, const Vector4& clearColor);
 
private:
	
	DirectXCommon* dxCommon_ = nullptr; // DirectX共通部分のポインタ
	WinApp* winApp_ = nullptr; // ウィンドウズアプリケーションのポインタ
	Microsoft::WRL::ComPtr <ID3D12Device> device; // デバイスのポインタ
	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList = nullptr;

	OffScreenRenderingPSO* offScreenRenderingPSO = nullptr; // OffScreenRenderingPSOクラスのポインタ
	
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> offscreenRtvDescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE offscreenRtvHandle{};

	const Vector4 kRenderTargetClearValue = { 1.0f,0.0f,0.0f,1.0f }; // わかりやすいように赤色でクリア
	// クラスメンバとして保持
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource_;
	D3D12_RESOURCE_STATES renderTextureState = D3D12_RESOURCE_STATE_RENDER_TARGET;
	D3D12_RESOURCE_BARRIER renderingBarrier{};
  
};
