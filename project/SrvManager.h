#pragma once
#include "DirectXCommon.h"

class SrvManager
{

public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectX共通部分</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// SRVの割り当て
	/// </summary>
	/// <returns></returns>
	uint32_t Allocate();

	/// <summary>
	/// ディスクリプタヒープのCPUハンドルを取得
	/// </summary>
	/// <param name="index">ディスクリプタヒープのインデックス</param>
	/// <param name="pResource">リソース</param>
	/// <param name="format">フォーマット</param>
	/// <param name="MipLevls">ミップマップレベル</param>
	void CreateSRVforTexture2D(uint32_t index, ID3D12Resource* pResource, DXGI_FORMAT format, UINT MipLevls);

	/// <summary>
	/// ディスクリプタヒープのCPUハンドルを取得
	/// </summary>
	/// <param name="index">ディスクリプタヒープのインデックス</param>
	/// <param name="pResource">リソース</param>
	/// <param name="numElements">要素数</param>
	/// <param name="strideInBytes">バイト数</param>
	void CreateSRVforStructuredBuffer(uint32_t index, ID3D12Resource* pResource, UINT numElements, UINT strideInBytes);

	/// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw();


	//-------------------Getter & Setter-------------------//
	/// <summary>
	/// ディスクリプタヒープのCPUハンドルを取得
	/// </summary>
	/// <param name="index">ディスクリプタヒープのインデックス</param>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);


	/// <summary>
	/// ディスクリプタヒープのGPUハンドルを取得
	/// </summary>
	/// <param name="index">ディスクリプタヒープのインデックス</param>
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);


	void SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, uint32_t srvIndex) {
		dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(rootParameterIndex, GetGPUDescriptorHandle(srvIndex));
	}

private:
	//DirectX共通部分
	DirectXCommon* dxCommon = nullptr;

	//最大SRV数(最大テクスチャ枚数)
	static const uint32_t kMaxSRVCount;
	//SRV用のディスクリプタサイズ
	uint32_t descriptorSize;
	//SRVヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	//次に使用するSRVのインデックス
	uint32_t useSrvIndex = 0;

};

