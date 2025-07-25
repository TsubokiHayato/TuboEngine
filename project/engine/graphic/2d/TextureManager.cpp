#include "TextureManager.h"
#include"StringUtility.h"
#include<iostream>


TextureManager* TextureManager::instance = nullptr;
uint32_t TextureManager::kSRVIndexTop = 1;
void TextureManager::Initialize() {
	
	
	textureDatas.reserve(SrvManager::kMaxSRVCount);
	directoryPath_ = "Resources/Textures/";
}
void TextureManager::LoadTexture(const std::string& filePath) {
	fullPath_ = directoryPath_ + filePath;

	// 読み込み済みテクスチャを検索
	if (textureDatas.contains(fullPath_)) {
		// 読み込み済みなら早期return
		return;
	}

	assert(textureDatas.size() < SrvManager::kMaxSRVCount);

	// テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(fullPath_);
	HRESULT hr;
	if (filePathW.ends_with(L".dds")) {
		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	} else {
		hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}
	if (FAILED(hr)) {
		std::cerr << "Failed to load texture from file: " << fullPath_ << " HRESULT: " << hr << std::endl;
		return;
	}

	// ミップマップの作成
	DirectX::ScratchImage mipImages;
	if (DirectX::IsCompressed(image.GetMetadata().format)) {
		// 圧縮テクスチャの場合はミップマップを生成しない
		mipImages = std::move(image);
	} else {
		// 非圧縮テクスチャの場合はミップマップを生成
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 4, mipImages);
		if (FAILED(hr)) {
			std::cerr << "Failed to generate mipmaps for texture: " << fullPath_ << " HRESULT: " << hr << std::endl;
			return;
		}
	}


	// 追加したテクスチャデータの参照を取得する
	TextureData& textureData = textureDatas[fullPath_];

	textureData.filePath = fullPath_;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = DirectXCommon::GetInstance()->CreateTextureResource(textureData.metadata);

	// テクスチャデータの要素番号をSRVのインデックスとする
	textureData.srvIndex = SrvManager::GetInstance()->Allocate();
	textureData.srvHandleCPU = SrvManager::GetInstance()->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = SrvManager::GetInstance()->GetGPUDescriptorHandle(textureData.srvIndex);

	// SRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;


	if (textureData.metadata.IsCubemap()) {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE; // キューブマップ
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.MipLevels = UINT_MAX; // 全てのミップレベルを使用
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f; // 最小LODクランプ値
	} else {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
		srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);

	}


	// SRVの生成
	Microsoft::WRL::ComPtr<ID3D12Device> device = DirectXCommon::GetInstance()->GetDevice();
	device->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = DirectXCommon::GetInstance()->UploadTextureData(textureData.resource, mipImages);

	DirectXCommon::GetInstance()->CommandExecution();
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath) {
	fullPath_ = directoryPath_ + filePath;
	auto it = textureDatas.find(fullPath_);
	if (it == textureDatas.end()) {
		std::cerr << "Texture not found: " << fullPath_ << std::endl;
	}
	assert(it != textureDatas.end());
	TextureData& textureData = it->second;
	return textureData.srvHandleGPU;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath) {
	fullPath_ = directoryPath_ + filePath;
	if (!textureDatas.contains(fullPath_)) {
		std::cerr << "Texture not found: " << fullPath_ << std::endl;
	}
	assert(textureDatas.contains(fullPath_));

	TextureData& textureData = textureDatas[fullPath_];
	return textureData.metadata;
}

uint32_t TextureManager::GetSrvIndex(const std::string& filePath) {
	fullPath_ = directoryPath_ + filePath;
	if (!textureDatas.contains(fullPath_)) {
		std::cerr << "Texture not found: " << fullPath_ << std::endl;
	}
	assert(textureDatas.contains(fullPath_));

	TextureData& textureData = textureDatas[fullPath_];
	return textureData.srvIndex;
}


TextureManager* TextureManager::GetInstance() {
	if (instance == nullptr) {
		instance = new TextureManager;
	}
	return instance;
}

void TextureManager::Finalize() {


	textureDatas.clear();

	delete instance;
	instance = nullptr;
}
