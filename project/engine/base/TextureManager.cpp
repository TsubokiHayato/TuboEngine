#include "TextureManager.h"
TextureManager* TextureManager::instance = nullptr;
uint32_t TextureManager::kSRVIndexTop = 1;
void TextureManager::Initialize(DirectXCommon* dxCommon,SrvManager* srvManager)
{
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
	textureDatas.reserve(SrvManager::kMaxSRVCount);
}

void TextureManager::LoadTexture(const std::string& filePath)
{

	//読み込み済みテクスチャを検索
	if (textureDatas.contains(filePath)) {
		//読み込み済みなら早期return
		return;
	}
	
	assert(textureDatas.size() < SrvManager::kMaxSRVCount);

	//テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミップマップの作成
	DirectX::ScratchImage mipImages;
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	
	//追加したテクスチャデータの参照を取得する
	TextureData& textureData = textureDatas[filePath];

	textureData.filePath = filePath;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = dxCommon_->CreateTextureResource(textureData.metadata);


	//テクスチャデータの要素番号をSRVのインデックスとする
	textureData.srvIndex = srvManager_->Allocate();
	textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);

	//SRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);

	//SRVの生成

	Microsoft::WRL::ComPtr <ID3D12Device> device = dxCommon_->GetDevice();

	device->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

	Microsoft::WRL::ComPtr< ID3D12Resource> intermediateResource = dxCommon_->UploadTextureData(textureData.resource, mipImages);
	
	dxCommon_->CommandExecution();

	
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath)
{
	auto it = textureDatas.find(filePath);
	assert(it != textureDatas.end());
	TextureData& textureData = it->second;
	return textureData.srvHandleGPU;


}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath)
{
	assert(textureDatas.contains(filePath));

	TextureData& textureData = textureDatas[filePath];
	return textureData.metadata;

}

uint32_t TextureManager::GetSrvIndex(const std::string& filePath)
{
	assert(textureDatas.contains(filePath));

	TextureData& textureData = textureDatas[filePath];
	return textureData.srvIndex;

}


TextureManager* TextureManager::GetInstance()
{
	if (instance == nullptr) {
		instance = new TextureManager;
	}
	return instance;
}

void TextureManager::Finalize()
{
	
	textureDatas.clear();

	delete instance;
	instance = nullptr;
}
