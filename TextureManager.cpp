#include "TextureManager.h"
TextureManager* TextureManager::instance = nullptr;

void TextureManager::Initialize()
{
	textureDatas.reserve(DirectXCommon::kMaxSRVCount);
}

void TextureManager::LoadTexture(const std::string& filePath)
{
	//テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミップマップの作成
	DirectX::ScratchImage mipImages;
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	//テクスチャデータを追加
	textureDatas.resize(textureDatas.size() + 1);
	//追加したテクスチャデータの参照を取得する
	TextureData& textureData = textureDatas.back();

	textureData.filePath = filePath;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = DirectXCommon::(textureData.metadata);
}

TextureManager* TextureManager::GetInstance()
{
	if (instance == nullptr) {
		instance = new TextureManager;
	}
	return nullptr;
}

void TextureManager::Finalize()
{
	delete instance;
	instance = nullptr;
}
