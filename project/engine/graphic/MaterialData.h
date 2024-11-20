#pragma once
#include <cstdint>
#include <string>

//マテリアルデータ
struct MaterialData {
	//テクスチャファイルパス
	std::string textureFilePath;
	//テクスチャ番号
	uint32_t textureIndex = 0;
};