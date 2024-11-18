#pragma once
#include"MaterialData.h"
#include"VertexData.h"
//モデルデータ
struct ModelData {
	//頂点データ
	std::vector<VertexData> vertices;
	//マテリアルデータ
	MaterialData material;
};