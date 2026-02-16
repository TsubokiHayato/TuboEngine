#include "Object3d.h"
#include "Camera.h"
#include "Matrix.h"
#include "Model.h"
#include "ModelManager.h"
#include "Object3dCommon.h"
#include "TextureManager.h"
#include "numbers"

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

void Object3d::Initialize(std::string modelFileNamePath) {
	
	this->camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();

	ModelManager::GetInstance()->LoadModel(modelFileNamePath);
	SetModel(modelFileNamePath);

#pragma region TransformMatrixResourced

	// WVP用のリソースを作る
	transformMatrixResource_ = TuboEngine::DirectXCommon::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));
	// データを書き込む
	transformMatrixData_ = nullptr;
	// 書き込むためのアドレスを取得
	transformMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformMatrixData_));
	// 単位行列を書き込んでいく
	transformMatrixData_->WVP = MakeIdentity4x4();
	transformMatrixData_->World = MakeIdentity4x4();

#pragma endregion TransformMatrixResource

#pragma region DirectionalLightData
	// 平行光源用用のリソースを作る。今回はColor1つ分のサイズを用意する
	directionalLightResource_ = TuboEngine::DirectXCommon::GetInstance()->CreateBufferResource(sizeof(DirectionalLight));
	// 平行光源用にデータを書き込む
	directionalLightData_ = nullptr;
	// 書き込むためのアドレスを取得
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));

	// デフォルト値
	directionalLightData_->color = {1.0f, 1.0f, 1.0f, 1.0f};
	directionalLightData_->direction = {0.0f, -1.0f, 0.0f};
	directionalLightData_->intensity = 1.0f;

#pragma endregion

#pragma region PointLight

	// ポイントライト用用のリソースを作る。今回はColor1つ分のサイズを用意する
	pointLightResource_ = TuboEngine::DirectXCommon::GetInstance()->CreateBufferResource(sizeof(PointLight));
	// 平行光源用にデータを書き込む
	pointLightData_ = nullptr;
	// 書き込むためのアドレスを取得
	pointLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData_));
	// デフォルト値
	pointLightData_->color = {1.0f, 1.0f, 1.0f, 1.0f};
	pointLightData_->position = {0.0f, 1.0f, 0.0f};
	pointLightData_->intensity = 1.0f;

#pragma endregion

#pragma region SpotLight

	// スポットライト用用のリソースを作る。今回はColor1つ分のサイズを用意する
	spotLightResource_ = TuboEngine::DirectXCommon::GetInstance()->CreateBufferResource(sizeof(SpotLight));
	// 平行光源用にデータを書き込む
	spotLightData_ = nullptr;
	// 書き込むためのアドレスを取得
	spotLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData_));
	// デフォルト値
	spotLightData_->color = {1.0f, 1.0f, 1.0f, 1.0f};
	spotLightData_->position = {2.0f, 1.25f, 0.0f};
	spotLightData_->direction = TuboEngine::Math::Vector3::Normalize({-1.0f, -1.0f, 0.0f});
	spotLightData_->intensity = 4.0f;
	spotLightData_->distance = 7.0f;
	spotLightData_->decay = 2.0f;
	spotLightData_->cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);

#pragma endregion

#pragma region cameraWorldPos
	// 平行光源用用のリソースを作る。今回はColor1つ分のサイズを用意する
	cameraForGPUResource_ = TuboEngine::DirectXCommon::GetInstance()->CreateBufferResource(sizeof(CameraForGPU));
	// 平行光源用にデータを書き込む
	cameraForGPUData_ = nullptr;
	// 書き込むためのアドレスを取得
	cameraForGPUResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPUData_));

	cameraForGPUData_->worldPosition = {};
#pragma endregion

#pragma region LightType
	// ライトの種類
	lightTypeResource_ = TuboEngine::DirectXCommon::GetInstance()->CreateBufferResource(sizeof(LightType));
	// 平行光源用にデータを書き込む
	lightTypeData_ = nullptr;
	// 書き込むためのアドレスを取得
	lightTypeResource_->Map(0, nullptr, reinterpret_cast<void**>(&lightTypeData_));

	// デフォルト値
	lightTypeData_->type = 0;

#pragma endregion

	// transform変数を作る
	transform_ = {
	    {1.0f, 1.0f, 1.0f},
	    {0.0f, 0.0f, 0.0f},
	    {0.0f, 0.0f, 0.0f},
	};
	cameraTransform_ = {
	    {1.0f, 1.0f, 1.0f  },
	    {0.3f, 0.0f, 0.0f  },
	    {0.0f, 4.0f, -10.0f},
	};
}

void Object3d::Update() {

	cameraForGPUData_->worldPosition = camera_->GetTranslate();

	// 行列を更新する
	TuboEngine::Math::Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	TuboEngine::Math::Matrix4x4 cameraMatrix = MakeAffineMatrix(camera_->GetScale(), camera_->GetRotation(), camera_->GetTranslate());
	TuboEngine::Math::Matrix4x4 viewMatrix = Inverse(cameraMatrix);
	TuboEngine::Math::Matrix4x4 projectionMatrix =
	    MakePerspectiveMatrix(0.45f, float(TuboEngine::WinApp::GetInstance()->GetClientWidth()) / float(TuboEngine::WinApp::GetInstance()->GetClientHeight()), 0.1f, 100.0f);
	TuboEngine::Math::Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

	if (camera_) {
		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	} else {
		worldViewProjectionMatrix = worldMatrix;
	}
	// 行列を更新する
	TuboEngine::Math::Matrix4x4 localMatrix = model_->GetRootNodeLocalMatrix();
	(void)localMatrix;
	transformMatrixData_->WVP = model_->GetRootNodeLocalMatrix() * worldMatrix * Multiply(viewMatrix, projectionMatrix);
	transformMatrixData_->World = model_->GetRootNodeLocalMatrix() * worldMatrix;

	commandList_ = TuboEngine::DirectXCommon::GetInstance()->GetCommandList();
}

void Object3d::Draw() {

	// TransformMatrix (b0, VertexShader)
	commandList_->SetGraphicsRootConstantBufferView(1, transformMatrixResource_->GetGPUVirtualAddress());

	// gTexture (t0, PixelShader)はModelクラスにある。
	
	// 例: キューブマップSRVをt1（gCubeTexture）にバインド
	// gCubeTexture (t1, PixelShader)
	commandList_->SetGraphicsRootDescriptorTable(3, TextureManager::GetInstance()->GetSrvHandleGPU(cubeMapFilePath_));
	// DirectionalLight (b1, PixelShader)
	commandList_->SetGraphicsRootConstantBufferView(4, directionalLightResource_->GetGPUVirtualAddress());
	// Camera (b2, PixelShader)
	commandList_->SetGraphicsRootConstantBufferView(5, cameraForGPUResource_->GetGPUVirtualAddress());
	// LightType (b3, PixelShader)
	commandList_->SetGraphicsRootConstantBufferView(6, lightTypeResource_->GetGPUVirtualAddress());
	// PointLight (b4, PixelShader)
	commandList_->SetGraphicsRootConstantBufferView(7, pointLightResource_->GetGPUVirtualAddress());
	// SpotLight (b5, PixelShader)
	commandList_->SetGraphicsRootConstantBufferView(8, spotLightResource_->GetGPUVirtualAddress());

	// 3Dモデルが割り当てられていれば描画
	if (model_) {
		model_->Draw();
	} else {
		// 3Dモデルが割り当てられていない場合はブレイクさせる
		assert(0);
	}
}

void Object3d::DrawImGui(const char* windowName) {
#ifdef USE_IMGUI
    ImGui::Begin(windowName);

    ImGui::Separator();
    ImGui::Text("Object3d ImGui コントロール");
    // 位置
    TuboEngine::Math::Vector3 pos = GetPosition();
    if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
        SetPosition(pos);
    }
    // 回転
    TuboEngine::Math::Vector3 rot = GetRotation();
    if (ImGui::DragFloat3("Rotation", &rot.x, 0.01f)) {
        SetRotation(rot);
    }
    // スケール
    TuboEngine::Math::Vector3 scale = GetScale();
    if (ImGui::DragFloat3("Scale", &scale.x, 0.01f)) {
        SetScale(scale);
    }
    // モデル色
    TuboEngine::Math::Vector4 color = GetModelColor();
    if (ImGui::ColorEdit4("Color", &color.x)) {
        SetModelColor(color);
    }

    // ライトタイプ選択
    static const char* lightTypeItems[] = {
        "Directional", "Phong", "Blinn-Phong", "Point", "Spot", "EnvironmentMap"
    };
    int lightType = GetLightType();
    if (ImGui::Combo("LightType", &lightType, lightTypeItems, IM_ARRAYSIZE(lightTypeItems))) {
        SetLightType(lightType);
    }

    // 必要なライトだけImGui表示
    if (lightType == 0) { // Directional
        ImGui::Separator();
        ImGui::Text("Directional Light");
        ImGui::ColorEdit4("LightColor", &directionalLightData_->color.x);
        ImGui::DragFloat3("LightDirection", &directionalLightData_->direction.x, 0.1f);
        ImGui::DragFloat("LightIntensity", &directionalLightData_->intensity, 0.1f);
        float shininess = model_->GetModelShininess();
        if (ImGui::DragFloat("Shininess", &shininess)) {
            model_->SetModelShininess(shininess);
        }
    } else if (lightType == 1) { // Phong
        ImGui::Separator();
        ImGui::Text("Phong Reflection");
        ImGui::ColorEdit4("LightColor", &directionalLightData_->color.x);
        ImGui::DragFloat3("LightDirection", &directionalLightData_->direction.x, 0.1f);
        ImGui::DragFloat("LightIntensity", &directionalLightData_->intensity, 0.1f);
        float shininess = model_->GetModelShininess();
        if (ImGui::DragFloat("Shininess", &shininess)) {
            model_->SetModelShininess(shininess);
        }
    } else if (lightType == 2) { // Blinn-Phong
        ImGui::Separator();
        ImGui::Text("Blinn-Phong Reflection");
        ImGui::ColorEdit4("LightColor", &directionalLightData_->color.x);
        ImGui::DragFloat3("LightDirection", &directionalLightData_->direction.x, 0.1f);
        ImGui::DragFloat("LightIntensity", &directionalLightData_->intensity, 0.1f);
        float shininess = model_->GetModelShininess();
        if (ImGui::DragFloat("Shininess", &shininess)) {
            model_->SetModelShininess(shininess);
        }
    } else if (lightType == 3) { // Point
        ImGui::Separator();
        ImGui::Text("Point Light");
        ImGui::ColorEdit4("PointLightColor", &pointLightData_->color.x);
        ImGui::DragFloat3("PointLightPosition", &pointLightData_->position.x, 0.1f);
        ImGui::DragFloat("PointLightIntensity", &pointLightData_->intensity, 0.1f);
    } else if (lightType == 4) { // Spot
        ImGui::Separator();
        ImGui::Text("Spot Light");
        ImGui::ColorEdit4("SpotLightColor", &spotLightData_->color.x);
        ImGui::DragFloat3("SpotLightPosition", &spotLightData_->position.x, 0.1f);
        ImGui::DragFloat3("SpotLightDirection", &spotLightData_->direction.x, 0.1f);
        ImGui::DragFloat("SpotLightIntensity", &spotLightData_->intensity, 0.1f);
        ImGui::DragFloat("SpotLightDistance", &spotLightData_->distance, 0.1f);
        ImGui::DragFloat("SpotLightDecay", &spotLightData_->decay, 0.1f);
        ImGui::DragFloat("SpotLightCosAngle", &spotLightData_->cosAngle, 0.1f);
    } else if (lightType == 5) { // EnvironmentMap
        ImGui::Separator();
        ImGui::Text("Environment Mapping");
        ImGui::ColorEdit4("LightColor", &directionalLightData_->color.x);
        ImGui::DragFloat3("LightDirection", &directionalLightData_->direction.x, 0.1f);
        ImGui::DragFloat("LightIntensity", &directionalLightData_->intensity, 0.1f);
        float shininess = model_->GetModelShininess();
        if (ImGui::DragFloat("Shininess", &shininess)) {
            model_->SetModelShininess(shininess);
        }
        float envCoef = model_->GetModelEnvironmentCoefficient();
        if (ImGui::SliderFloat("Environment Coefficient", &envCoef, 0.0f, 1.0f)) {
            model_->SetModelEnvironmentCoefficient(envCoef);
        }
    }
    ImGui::End();
#endif // USE_IMGUI
}

void Object3d::SetLightShininess(float shininess) { model_->SetModelShininess(shininess); }

void Object3d::SetModel(const std::string& filePath) { model_ = ModelManager::GetInstance()->FindModel(filePath); }

void Object3d::SetModelColor(const Vector4& color) { model_->SetModelColor(color); }

Vector4 Object3d::GetModelColor() { return model_->GetModelColor(); }

float Object3d::GetLightShininess() { return model_->GetModelShininess(); }
