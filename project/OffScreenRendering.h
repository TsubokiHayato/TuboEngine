#pragma once

#include <wrl.h>
#include <d3d12.h>
#include "Vector4.h"

class OffScreenRendering
{
public:
   
	/// <summary>
	/// コンストラクタ
	/// </summary>
	OffScreenRendering() = default;
	/// <summary>
	/// デストラクタ
	/// </summary>
	~OffScreenRendering() = default;
	/// <summary>
	/// レンダーテクスチャの作成
	/// </summary>
	void CreateRenderTargetResource(Microsoft::WRL::ComPtr<ID3D12Device>& device, int32_t width, int32_t height, DXGI_FORMAT format, const Vector4& clearColor);
	/// <summary>
	/// レンダーテクスチャのクリア
	/// </summary>
	void ClearRenderTargetPreDraw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, Microsoft::WRL::ComPtr<ID3D12Resource>& renderTarget);

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> renderTexture_; // RenderTexture
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_; // RTV用のディスクリプタヒープ
    Microsoft::WRL::ComPtr<ID3D12Resource> depthBuffer_;   // 深度バッファ
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_; // DSV用のディスクリプタヒープ
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_;               // RTVハンドル
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_;               // DSVハンドル
    Vector4 clearColor_;                                  // クリアカラー
    int width_;                                           // レンダーテクスチャの幅
    int height_;                                          // レンダーテクスチャの高さ
};
