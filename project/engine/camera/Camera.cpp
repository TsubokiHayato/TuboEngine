#include "Camera.h"
#include"WinApp.h"
#include"MT_Matrix.h"
Camera::Camera()

	:transform_({ { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }, { 1.0f,1.0f,1.0f } })
	, fovY_(0.45f)
	, aspect_(float(WinApp::GetInstance()->GetClientWidth()) / float(WinApp::GetInstance()->GetClientHeight()))
	, nearClip_(0.1f)
	, farClip_(100.0f)
	, worldMatrix_(MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate))
	, viewMatrix_(Inverse(worldMatrix_))
	, projectionMatrix_(MakePerspectiveMatrix(fovY_, aspect_, nearClip_, farClip_))
	, viewProjectionMatrix_(Multiply(viewMatrix_, projectionMatrix_))
{
	// 例: BlenderのカメラFOVが50度の場合
	constexpr float kBlenderFovY = 50.0f * 3.14159265358979323846f / 180.0f; // 50度→ラジアン
	fovY_ = kBlenderFovY;
}


void Camera::Update()
{
	//ワールド行列の作成
	worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	//ビュー行列の作成
	viewMatrix_ = Inverse(worldMatrix_);
	//プロジェクション行列の作成
	projectionMatrix_ = MakePerspectiveMatrix(fovY_, aspect_, nearClip_, farClip_);
	//ビュープロジェクション行列の作成
	viewProjectionMatrix_ = Multiply(viewMatrix_, projectionMatrix_);
	
}
