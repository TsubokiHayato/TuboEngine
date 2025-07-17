#pragma once
#include "Vector3.h"

//座標
struct EulerTransform {
	//拡大率
	Vector3 scale;
	//Eulerでの回転
	Vector3 rotate;
	//平行移動
	Vector3 translate;
};