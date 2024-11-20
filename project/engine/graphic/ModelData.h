#pragma once
#include"MaterialData.h"
#include"VertexData.h"
#include<vector>
//モデルデータ
struct ModelData {
	//頂点データ
	std::vector<VertexData> vertices;
	//マテリアルデータ
	MaterialData material;
};