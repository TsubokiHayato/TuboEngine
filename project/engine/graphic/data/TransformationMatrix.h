#pragma once
#include "Matrix4x4.h"

//変換行列
struct  TransformationMatrix {
	//ワールドビュープロジェクション行列
	TuboEngine::Math::Matrix4x4 WVP;
	//ワールド行列
	TuboEngine::Math::Matrix4x4 World;
};