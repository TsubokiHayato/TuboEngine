#include "SrvManager.h"

const uint32_t SrvManager::kMaxSRVCount = 1024;

void SrvManager::Initialize(DirectXCommon* dxCommon)
{
	//DirectX共通部分の設定
	this->dxCommon_ = dxCommon;
	//ディスクリプタヒープの生成
	descriptorHeap = this->dxCommon_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);
	//ディスクリプタサイズの取得
	descriptorSize = this->dxCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// 初期化時に全てのインデックスを空きリストに追加
	for (uint32_t i = 0; i < kMaxSRVCount; ++i) {
		freeIndices.push_back(i);
	}

}

uint32_t SrvManager::Allocate()
{
	assert(!freeIndices.empty() && "No available SRV indices!");

	// 空きリストからインデックスを取り出す
	uint32_t index = freeIndices.back();
	freeIndices.pop_back();
	allocatedIndices.insert(index);

	return index;
}

void SrvManager::Free(uint32_t index)
{
	assert(allocatedIndices.find(index) != allocatedIndices.end() && "Index not allocated!");

	// 確保済みインデックスを空きリストに戻す
	allocatedIndices.erase(index);
	freeIndices.push_back(index);
}

void SrvManager::CreateSRVforTexture2D(uint32_t index, ID3D12Resource* pResource, DXGI_FORMAT format, UINT MipLevls)
{

	//ディスクリプタヒープのCPUハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = GetCPUDescriptorHandle(index);
	//SRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = MipLevls;
	//SRVの作成
	dxCommon_->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, handleCPU);
}

void SrvManager::CreateSRVforStructuredBuffer(uint32_t index, ID3D12Resource* pResource, UINT numElements, UINT strideInBytes)
{
	//ディスクリプタヒープのCPUハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = GetCPUDescriptorHandle(index);
	//SRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = numElements;
	srvDesc.Buffer.StructureByteStride = strideInBytes;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	//SRVの作成
	dxCommon_->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, handleCPU);
}

void SrvManager::PreDraw()
{
	//ディスクリプタヒープのインデックスをリセット
	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap.Get() };
	//SRV用のディスクリプタヒープを指定する
	dxCommon_->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
}

bool SrvManager::CheckTextureCount(uint32_t count)
{
	//テクスチャ枚数が上限を超えていないかチェック
	if (useSrvIndex + count > kMaxSRVCount) {
		//上限を超えていたらfalseを返す
		return false;
	}
	//上限を超えていなかったらtrueを返す
	return true;
}

D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index)
{
	//CPUハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//インデックス分ずらす
	handleCPU.ptr += (descriptorSize * index);
	//CPUハンドルを返す
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index)
{
	//GPUハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	//インデックス分ずらす
	handleGPU.ptr += (descriptorSize * index);
	//GPUハンドルを返す
	return handleGPU;
}
