#pragma once
#include <cstdint>
#include "Vector4.h"
#include "Matrix4x4.h"

//マテリアル
struct Material {
	//	色
	Vector4 color;
	//ライティングを有効にするか
	int32_t enableLighting;
	//パディング
	float padding[3];
	//UV変換行列
	Matrix4x4 uvTransform;
	//光沢度
	float shininess;
};