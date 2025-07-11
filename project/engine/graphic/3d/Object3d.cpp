#include "Object3d.h"
#include "Camera.h"
#include "MT_Matrix.h"
#include "Model.h"
#include "ModelManager.h"
#include "Object3dCommon.h"
#include "TextureManager.h"
#include "numbers"

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

void Object3d::Initialize(std::string modelFileNamePath) {
	// 引数で受け取ってメンバ変数に記録する
	
	this->camera = Object3dCommon::GetInstance()->GetDefaultCamera();

	ModelManager::GetInstance()->LoadModel(modelFileNamePath);

#pragma region TransformMatrixResourced

	// WVP用のリソースを作る
	transformMatrixResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));
	// データを書き込む
	transformMatrixData = nullptr;
	// 書き込むためのアドレスを取得
	transformMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformMatrixData));
	// 単位行列を書き込んでいく
	transformMatrixData->WVP = MakeIdentity4x4();
	transformMatrixData->World = MakeIdentity4x4();

#pragma endregion TransformMatrixResource

#pragma region DirectionalLightData
	// 平行光源用用のリソースを作る。今回はColor1つ分のサイズを用意する
	directionalLightResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(DirectionalLight));
	// 平行光源用にデータを書き込む
	directionalLightData = nullptr;
	// 書き込むためのアドレスを取得
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));

	// デフォルト値
	directionalLightData->color = {1.0f, 1.0f, 1.0f, 1.0f};
	directionalLightData->direction = {0.0f, -1.0f, 0.0f};
	directionalLightData->intensity = 1.0f;

#pragma endregion

#pragma region PointLight

	// ポイントライト用用のリソースを作る。今回はColor1つ分のサイズを用意する
	pointLightResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(PointLight));
	// 平行光源用にデータを書き込む
	pointLightData = nullptr;
	// 書き込むためのアドレスを取得
	pointLightResource->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData));
	// デフォルト値
	pointLightData->color = {1.0f, 1.0f, 1.0f, 1.0f};
	pointLightData->position = {0.0f, 1.0f, 0.0f};
	pointLightData->intensity = 1.0f;

#pragma endregion

#pragma region SpotLight

	// スポットライト用用のリソースを作る。今回はColor1つ分のサイズを用意する
	spotLightResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(SpotLight));
	// 平行光源用にデータを書き込む
	spotLightData = nullptr;
	// 書き込むためのアドレスを取得
	spotLightResource->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData));
	// デフォルト値
	spotLightData->color = {1.0f, 1.0f, 1.0f, 1.0f};
	spotLightData->position = {2.0f, 1.25f, 0.0f};
	spotLightData->direction = Vector3::Normalize({-1.0f, -1.0f, 0.0f});
	spotLightData->intensity = 4.0f;
	spotLightData->distance = 7.0f;
	spotLightData->decay = 2.0f;
	spotLightData->cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);

#pragma endregion

#pragma region cameraWorldPos
	// 平行光源用用のリソースを作る。今回はColor1つ分のサイズを用意する
	cameraForGPUResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(CameraForGPU));
	// 平行光源用にデータを書き込む
	cameraForGPUData = nullptr;
	// 書き込むためのアドレスを取得
	cameraForGPUResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPUData));

	cameraForGPUData->worldPosition = {};
#pragma endregion

#pragma region LightType
	// ライトの種類
	lightTypeResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(LightType));
	// 平行光源用にデータを書き込む
	lightTypeData = nullptr;
	// 書き込むためのアドレスを取得
	lightTypeResource->Map(0, nullptr, reinterpret_cast<void**>(&lightTypeData));

	// デフォルト値
	lightTypeData->type = 0;

#pragma endregion



	// transform変数を作る
	transform = {
	    {1.0f, 1.0f, 1.0f},
	    {0.0f, 0.0f, 0.0f},
	    {0.0f, 0.0f, 0.0f},
	};
	cameraTransform = {
	    {1.0f, 1.0f, 1.0f  },
	    {0.3f, 0.0f, 0.0f  },
	    {0.0f, 4.0f, -10.0f},
	};
}

void Object3d::Update() {

	cameraForGPUData->worldPosition = camera->GetTranslate();

	// 行列を更新する
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 cameraMatrix = MakeAffineMatrix(camera->GetScale(), camera->GetRotation(), camera->GetTranslate());
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = MakePerspectiveMatrix(0.45f, float(WinApp::GetInstance()->GetClientWidth()) / float(WinApp::GetInstance()->GetClientHeight()), 0.1f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

	if (camera) {
		const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	} else {
		worldViewProjectionMatrix = worldMatrix;
	}
	// 行列を更新する
	Matrix4x4 localMatrix = model_->GetRootNodeLocalMatrix();
	transformMatrixData->WVP = model_->GetRootNodeLocalMatrix() * worldMatrix * Multiply(viewMatrix, projectionMatrix);
	transformMatrixData->World = model_->GetRootNodeLocalMatrix() * worldMatrix;

	commandList = DirectXCommon::GetInstance()->GetCommandList();
}

void Object3d::Draw() {

	// TransformMatrix (b0, VertexShader)
	commandList->SetGraphicsRootConstantBufferView(1, transformMatrixResource->GetGPUVirtualAddress());

	// gTexture (t0, PixelShader)はModelクラスにある。
	
	// 例: キューブマップSRVをt1（gCubeTexture）にバインド
	// gCubeTexture (t1, PixelShader)
	commandList->SetGraphicsRootDescriptorTable(3, TextureManager::GetInstance()->GetSrvHandleGPU(cubeMapFilePath_));
	// DirectionalLight (b1, PixelShader)
	commandList->SetGraphicsRootConstantBufferView(4, directionalLightResource->GetGPUVirtualAddress());
	// Camera (b2, PixelShader)
	commandList->SetGraphicsRootConstantBufferView(5, cameraForGPUResource->GetGPUVirtualAddress());
	// LightType (b3, PixelShader)
	commandList->SetGraphicsRootConstantBufferView(6, lightTypeResource->GetGPUVirtualAddress());
	// PointLight (b4, PixelShader)
	commandList->SetGraphicsRootConstantBufferView(7, pointLightResource->GetGPUVirtualAddress());
	// SpotLight (b5, PixelShader)
	commandList->SetGraphicsRootConstantBufferView(8, spotLightResource->GetGPUVirtualAddress());

	


	// 3Dモデルが割り当てられていれば描画
	if (model_) {
		model_->Draw();
	} else {
		// 3Dモデルが割り当てられていない場合はブレイクさせる
		assert(0);
	}
}

void Object3d::ShowImGuiLight() {


	ImGui::Begin("Model");
	float environmentCoefficient = model_->GetModelEnvironmentCoefficient();
	ImGui::DragFloat("environmentCoefficient", &environmentCoefficient, 0.1f, 0.0f, 1.0f);
	model_->SetModelEnvironmentCoefficient(environmentCoefficient);
	ImGui::End();


	ImGui::Begin("Light");
	// 光源のタイプ

	ImGui::SliderInt("LightType", &lightTypeData->type, 0, 5);
	// 光沢度
	float shininess = model_->GetModelShininess();
	ImGui::DragFloat("Shininess", &shininess);
	model_->SetModelShininess(shininess);

	// 光源の色

	ImGui::ColorEdit4("LightColor", &directionalLightData->color.x);

	// 光源の方向

	ImGui::DragFloat3("LightDirection", &directionalLightData->direction.x, 0.1f);

	// 光源の強さ

	ImGui::DragFloat("LightIntensity", &directionalLightData->intensity, 0.1f);

	ImGui::Text("PointLight");
	// 光源の色

	ImGui::ColorEdit4("PointLightColor", &pointLightData->color.x);

	// 光源の位置
	ImGui::DragFloat3("PointLightPosition", &pointLightData->position.x, 0.1f);

	// 光源の強さ
	ImGui::DragFloat("PointLightIntensity", &pointLightData->intensity, 0.1f);

	ImGui::Text("SpotLight");
	// 光源の色

	ImGui::ColorEdit4("SpotLightColor", &spotLightData->color.x);

	// 光源の位置

	ImGui::DragFloat3("SpotLightPosition", &spotLightData->position.x, 0.1f);

	// 光源の方向

	ImGui::DragFloat3("SpotLightDirection", &spotLightData->direction.x, 0.1f);

	// 光源の強さ

	ImGui::DragFloat("SpotLightIntensity", &spotLightData->intensity, 0.1f);

	// 光源の距離
	ImGui::DragFloat("SpotLightDistance", &spotLightData->distance, 0.1f);

	// 光源の減衰

	ImGui::DragFloat("SpotLightDecay", &spotLightData->decay, 0.1f);

	// 光源の角度

	ImGui::DragFloat("SpotLightCosAngle", &spotLightData->cosAngle, 0.1f);

	ImGui::End();
}

void Object3d::SetLightShininess(float shininess) { model_->SetModelShininess(shininess); }

void Object3d::SetModel(const std::string& filePath) { model_ = ModelManager::GetInstance()->FindModel(filePath); }

void Object3d::SetModelColor(const Vector4& color) { model_->SetModelColor(color); }

Vector4 Object3d::GetModelColor() { return model_->GetModelColor(); }

float Object3d::GetLightShininess() { return model_->GetModelShininess(); }
