#pragma once
#include "Vector3.h"
#include "Quaternion.h"
struct QuaternionTransform {
	Vector3 scale; // 拡大率
	Quaternion rotate; // Quaternionでの回転
	Vector3 translate; // 平行移動
};
