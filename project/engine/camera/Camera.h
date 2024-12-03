#pragma once
#include"Transform.h"
#include"MT_Matrix.h"
class Camera
{
public:

	/// <summary>
	///	更新処理
	/// </summary>
	void Update();
	
private:
	Transform transform_;//座標変換
	Matrix4x4 worldMatrix_;//ワールド行列
	Matrix4x4 viewMatrix_;//ビュー行列
	
	Matrix4x4 projectionMatrix_;//プロジェクション行列
	
};

