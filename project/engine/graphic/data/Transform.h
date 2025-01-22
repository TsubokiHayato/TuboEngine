#pragma once
#include "Vector3.h"

//座標
struct Transform {
	//拡大率
	Vector3 scale;
	//回転
	Vector3 rotate;
	//平行移動
	Vector3 translate;
};