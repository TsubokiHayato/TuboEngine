#include "TitleScene.h"
#include"TextureManager.h"
#include"ImGuiManager.h"
#include"numbers"
#include"Input.h"
#include"SceneManager.h"
#include"LineManager.h"
#include <cmath> // 追加

void TitleScene::Initialize() {

	//カメラ
	camera = std::make_unique<Camera>();
	camera->SetTranslate({ 640.0f, 360.0f, -1000.0f });
	camera->setRotation({ 0.0f, 0.0f, 0.0f });
	camera->setScale({ 1.0f, 1.0f, 1.0f });

	titleUI = std::make_unique<TitleUI>();
	titleUI->Initialize();

}

void TitleScene::Update() {
	//カメラ
	
	camera->Update();

	//if (Input::GetInstance()->PushKey(DIK_SPACE)) {
	////スペースキーが押されたらシーンを切り替える
	//	ChangeNextScene(STAGE);
	//}

	titleUI->Update();
	
	LineManager::GetInstance()->SetDefaultCamera(camera.get());
	LineManager::GetInstance()->Update();
}

void TitleScene::Finalize() {


}

void TitleScene::Object3DDraw() {

// ==== 背景アニメーション ====
// サイン波状のラインを複数描画
const int lineCount = 8;
const float width = 1280.0f;
const float height = 720.0f;
const float amplitude = 40.0f;
const float frequency = 0.015f;
const float speed = 1.0f;
float time = static_cast<float>(std::fmod(ImGui::GetTime(), 10000.0)); // 時間経過

// 通常方向
for (int i = 0; i < lineCount; ++i) {
    float baseY = 80.0f + i * (height - 160.0f) / (lineCount - 1);
    Vector3 prev = {0.0f, baseY, 0.0f};
    for (int x = 1; x <= 128; ++x) {
        float fx = width * x / 128.0f;
        float offset = amplitude * std::sin(fx * frequency + time * speed + i);
        Vector3 curr = {fx, baseY + offset, 0.0f};
        // 色をグラデーションに
        float t = static_cast<float>(i) / (lineCount - 1);
        Vector4 color = {0.2f + 0.6f * t, 0.5f + 0.5f * std::sin(time + i), 1.0f - t, 1.0f};
        LineManager::GetInstance()->DrawLine(prev, curr, color);
        prev = curr;
    }
}

// 反対方向
for (int i = 0; i < lineCount; ++i) {
    float baseY = 80.0f + i * (height - 160.0f) / (lineCount - 1);
    Vector3 prev = {width, baseY, 0.0f};
    for (int x = 1; x <= 128; ++x) {
        float fx = width - (width * x / 128.0f);
        float offset = amplitude * std::sin(fx * frequency - time * speed - i); // 時間・位相を逆転
        Vector3 curr = {fx, baseY + offset, 0.0f};
        float t = static_cast<float>(i) / (lineCount - 1);
        Vector4 color = {1.0f - t, 0.5f + 0.5f * std::sin(-time + i), 0.2f + 0.6f * t, 0.7f}; // 色も少し変化
        LineManager::GetInstance()->DrawLine(prev, curr, color);
        prev = curr;
    }
}
}

void TitleScene::SpriteDraw() {
    

    // ==== UI描画 ====
    titleUI->Draw();
}

void TitleScene::ImGuiDraw() {

#ifdef USE_IMGUI
	//カメラ
	ImGui::Begin("camera");
	ImGui::DragFloat3("Position", &cameraPosition.x,0.01f);
	ImGui::DragFloat3("Rotation", &cameraRotation.x,0.01f);
	ImGui::DragFloat3("Scale", &cameraScale.x,0.01f);
	ImGui::End();

	LineManager::GetInstance()->DrawImGui();
}

void TitleScene::ParticleDraw() {
	//パーティクル
	particle->Draw();

}
