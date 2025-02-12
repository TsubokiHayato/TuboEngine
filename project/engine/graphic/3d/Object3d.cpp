#include "Object3d.h"
#include"Object3dCommon.h"
#include"ModelCommon.h"
#include"Model.h"
#include"TextureManager.h"
#include"ModelManager.h"
#include"Camera.h"
#include"MT_Matrix.h"


#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/imgui/imgui_impl_dx12.h"

void Object3d::Initialize(Object3dCommon* object3dCommon) {
	//引数で受け取ってメンバ変数に記録する
	this->object3dCommon = object3dCommon;
	this->dxCommon_ = object3dCommon->GetDxCommon();
	this->winApp_ = object3dCommon->GetWinApp();
	this->camera = object3dCommon->GetDefaultCamera();

#pragma region TransformMatrixResourced

	//WVP用のリソースを作る
	transformMatrixResource = this->dxCommon_->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込む
	transformMatrixData = nullptr;
	//書き込むためのアドレスを取得
	transformMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformMatrixData));
	//単位行列を書き込んでいく
	transformMatrixData->WVP = MakeIdentity4x4();
	transformMatrixData->World = MakeIdentity4x4();


#pragma endregion TransformMatrixResource


#pragma region DirectionalLightData
	//平行光源用用のリソースを作る。今回はColor1つ分のサイズを用意する
	directionalLightResource =
		this->dxCommon_->CreateBufferResource(sizeof(DirectionalLight));
	//平行光源用にデータを書き込む
	directionalLightData = nullptr;
	//書き込むためのアドレスを取得
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));

	//デフォルト値
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightData->intensity = 1.0f;

#pragma endregion


#pragma region cameraWorldPos
	//平行光源用用のリソースを作る。今回はColor1つ分のサイズを用意する
	cameraForGPUResource =
		this->dxCommon_->CreateBufferResource(sizeof(CameraForGPU));
	//平行光源用にデータを書き込む
	cameraForGPUData = nullptr;
	//書き込むためのアドレスを取得
	cameraForGPUResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPUData));

	cameraForGPUData->worldPosition = {};
#pragma endregion

	//ライトの種類
	lightTypeResource =
		this->dxCommon_->CreateBufferResource(sizeof(LightType));
	//平行光源用にデータを書き込む
	lightTypeData = nullptr;
	//書き込むためのアドレスを取得
	lightTypeResource->Map(0, nullptr, reinterpret_cast<void**>(&lightTypeData));

	//デフォルト値
	lightTypeData->type = 0;


	//transform変数を作る
	transform = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};
	cameraTransform = {
		{1.0f,1.0f,1.0f},
		{0.3f,0.0f,0.0f},
		{0.0f,4.0f,-10.0f},
	};


}

void Object3d::Update() {

	cameraForGPUData->worldPosition = camera->GetTranslate();

	//行列を更新する
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 cameraMatrix = MakeAffineMatrix(camera->GetScale(), camera->GetRotation(), camera->GetTranslate());
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = MakePerspectiveMatrix(0.45f, float(winApp_->kClientWidth) / float(winApp_->kClientHeight), 0.1f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

	if (camera) {
		const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	} else {
		worldViewProjectionMatrix = worldMatrix;
	}
	//行列を更新する
	transformMatrixData->WVP = worldViewProjectionMatrix;
	transformMatrixData->World = worldMatrix;

	//
	commandList = dxCommon_->GetCommandList();
}

void Object3d::Draw() {

	//wvp用のCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(1, transformMatrixResource->GetGPUVirtualAddress());
	//平行光源用のCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(4, cameraForGPUResource->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(5, lightTypeResource->GetGPUVirtualAddress());

	//3Dモデルが割り当てられていれば描画
	if (model_) {
		model_->Draw();
	}

}

void Object3d::SetLightShininess(float shininess) { model_->SetModelShininess(shininess); }

void Object3d::SetModel(const std::string& filePath) {
	model_ = ModelManager::GetInstance()->FindModel(filePath);
}

void Object3d::SetModelColor(const Vector4& color) {
	model_->SetModelColor(color);
}


Vector4 Object3d::GetModelColor() {
	return model_->GetModelColor();
}

float Object3d::GetLightShininess() {
	return	model_->GetModelShininess();
}

