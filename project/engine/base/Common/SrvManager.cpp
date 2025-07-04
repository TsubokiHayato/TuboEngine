#include "SrvManager.h"
#include <iostream>
#include <cassert>

const uint32_t SrvManager::kMaxSRVCount = 1024;

void SrvManager::Initialize()
{
	
	//ディスクリプタヒープの生成
	descriptorHeap = DirectXCommon::GetInstance()->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);
	//ディスクリプタサイズの取得
	descriptorSize = DirectXCommon::GetInstance()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);



}

uint32_t SrvManager::Allocate()
{

	// returnする番号を一旦記録
	uint32_t index = useIndex;
	//次に使用するディスクリプタのインデックスを進める
	useIndex++;
	//上で記録した番号を返す
	return index;
}
void SrvManager::CreateSRVforTexture2D(uint32_t srvIndex, Microsoft::WRL::ComPtr<ID3D12Resource> pResource, DXGI_FORMAT format, UINT mipLevels) {
	//========================================
	// ディスクリプタハンドルの取得
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * srvIndex);
	//========================================
	// テクスチャ用のSRVを生成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = mipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(pResource.Get(), &srvDesc, handleCPU);
}

///=============================================================================
///						SRV生成(構造化バッファ用)
void SrvManager::CreateSRVForStructuredBuffer(uint32_t index, Microsoft::WRL::ComPtr<ID3D12Resource> pResource, UINT elements, UINT structureByteStride) {
	//========================================
	// ディスクリプタハンドルの取得
	//D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	//handleCPU.ptr += ( descriptorSizeSRV_ * srvIndex );
	//========================================
	// 構造化バッファ用のSRVを生成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.StructureByteStride = structureByteStride;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = elements;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(pResource.Get(), &srvDesc, GetCPUDescriptorHandle(index));
}
void SrvManager::PreDraw()
{
	//ディスクリプタヒープのインデックスをリセット
	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap.Get() };
	//SRV用のディスクリプタヒープを指定する
	DirectXCommon::GetInstance()->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
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
