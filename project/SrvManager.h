#pragma once
#include "DirectXCommon.h"
#include <set>

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
	/// SRVの解放
	/// </summary>
	/// <param name="index">解放するSRVのインデックス</param>
	void Free(uint32_t index);


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


	/// <summary>
	///テクスチャ枚数上限チェック
	/// </summary>
	bool CheckTextureCount(uint32_t count);

	//-------------------Getter & Setter-------------------//
	/// <summary>
	/// ディスクリプタヒープのCPUハンドルを取得
	/// </summary>
	/// <param name="index">ディスクリプタヒープのインデックス</param>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);

	ID3D12DescriptorHeap* GetDescriptorHeap()const { return descriptorHeap.Get(); }
	/// <summary>
	/// ディスクリプタヒープのGPUハンドルを取得
	/// </summary>
	/// <param name="index">ディスクリプタヒープのインデックス</param>
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	void SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, uint32_t srvIndex) {
		dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(rootParameterIndex, GetGPUDescriptorHandle(srvIndex));
	}

	//最大SRV数(最大テクスチャ枚数)
	static const uint32_t kMaxSRVCount;

private:

	//DirectX共通部分
	DirectXCommon* dxCommon_ = nullptr;



	//SRV用のディスクリプタサイズ
	uint32_t descriptorSize;
	//SRVヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	//次に使用するSRVのインデックス
	uint32_t useSrvIndex = 0;


	std::vector<uint32_t> freeIndices; // 空きインデックスのリスト
	std::set<uint32_t> allocatedIndices; // 確保済みインデックスのセット


};

