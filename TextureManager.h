#pragma once
#include<string>
#include"externals/DirectXTex/DirectXTex.h"
#include"DirectXcommon.h"
class TextureManager
{
private:


	static TextureManager* instance;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;

	//テクスチャ1枚分のデータ
	struct TextureData
	{
		std::string filePath;//画像のファイルパス
		DirectX::TexMetadata metadata;//画像の高さなどの情報
		Microsoft::WRL::ComPtr<ID3D12Resource>resource;//テクスチャリソース
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;//SRV作成時に必要なCPUハンドル
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;//描画コマンドに必要なGPUハンドル


	};
	//テクスチャデータ
	std::vector<TextureData>textureDatas;

public:
	//シングルトンインスタンスの取得
	static TextureManager* GetInstance();
	//終了
	void Finalize();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	void LoadTexture(const std::string& filePath);
};

