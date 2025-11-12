#include "TitleScene.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "LineManager.h"
#include "SceneManager.h"
#include "TextureManager.h"
#include "numbers"
#include <algorithm> // std::clamp用
#include <cmath>     // 追加

void TitleScene::Initialize() {

	// カメラ
	camera = std::make_unique<Camera>();
	camera->SetTranslate({640.0f, 360.0f, -1000.0f});
	camera->setRotation({0.0f, 0.0f, 0.0f});
	camera->setScale({1.0f, 1.0f, 1.0f});

	titleUI = std::make_unique<TitleUI>();
	titleUI->Initialize();

	// フェード用スプライト生成
	fadeSprite_ = std::make_unique<Sprite>();
	fadeSprite_->Initialize("uvChecker.png");
	fadeSprite_->SetPosition({0.0f, 0.0f});
	fadeSprite_->SetSize({1280.0f, 720.0f}); // 画面サイズに合わせる
	fadeSprite_->SetAnchorPoint({0.0f, 0.0f});
	fadeSprite_->SetColor({0, 0, 0, 0}); // 初期は透明
	fadeSprite_->Update();
	// シーンチェンジアニメーション初期化（シーン開始時はDisappearingで覆いを消す）
	sceneChangeAnimation = std::make_unique<SceneChangeAnimation>(1280, 720, 80, 1.5f, "barrier.png");
	sceneChangeAnimation->Initialize();
	isRequestSceneChange = false;
}

void TitleScene::Update() {
	camera->Update();
	titleUI->Update();

	// 背景アニメーション用タイマーを加算
	time_ += 1.0f / 60.0f; // 仮に60FPS前提。可変フレームならdtを使う

	// フェードイン・アウトアニメーション更新
	if (isSceneChanging_) {
		float alpha = 0;
		alpha += 0.05f; // フェードアウト速度
		fadeSprite_->SetColor({0, 0, 0, alpha});
		fadeSprite_->Update();
	}


	LineManager::GetInstance()->SetDefaultCamera(camera.get());
	LineManager::GetInstance()->Update();

	// スペースキーで覆いを出すリクエスト
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		sceneChangeAnimation->SetPhase(SceneChangeAnimation::Phase::Appearing); // 覆いを出す
		isRequestSceneChange = true;
	// シーンチェンジ要求が来たらアニメーション開始
	if (titleUI->GetrRequestSceneChange_() && !isSceneChanging_) {
		isSceneChanging_ = true;
		sceneChangeTimer_ = 0.0f;
	}

	sceneChangeAnimation->Update(1.0f / 60.0f);

	// アニメーションが終わったらシーン遷移
	// スペースキーで覆いを出すリクエスト
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		// アニメーション中は新たなアニメーションを開始しない
		if (sceneChangeAnimation->IsFinished()) {
			sceneChangeAnimation->SetPhase(SceneChangeAnimation::Phase::Appearing);
			isRequestSceneChange = true;
		}
	}
}
	// シーンチェンジアニメーション進行
	if (isSceneChanging_) {
		sceneChangeTimer_ += 1.0f / 60.0f; // 仮に60FPS前提
		if (sceneChangeTimer_ >= sceneChangeDuration_) {
			// アニメーション終了後にシーン遷移
			//ChangeNextScene(STAGE);
		}
		return; // アニメーション中は他の処理をスキップ
	}
}

void TitleScene::Finalize() {}

void TitleScene::Object3DDraw() {

	// ==== 背景アニメーション ====
	// サイン波状のラインを複数描画
	const int lineCount = 8;
	const float width = 1280.0f;
	const float height = 720.0f;
	const float amplitude = 40.0f;
	const float frequency = 0.015f;
	const float speed = 1.0f;
	float time = std::fmod(time_, 10000.0f); // 独自タイマーに変更

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

	// アニメーション描画
	if (sceneChangeAnimation) {
		sceneChangeAnimation->Draw();
	}
}

void TitleScene::ImGuiDraw() {

#ifdef USE_IMGUI
	//カメラ
	ImGui::Begin("camera");
	ImGui::DragFloat3("Position", &cameraPosition.x, 0.01f);
	ImGui::DragFloat3("Rotation", &cameraRotation.x, 0.01f);
	ImGui::DragFloat3("Scale", &cameraScale.x, 0.01f);
	ImGui::End();

	if (sceneChangeAnimation) {
		sceneChangeAnimation->DrawImGui();
	}
#endif // USE_IMGUI
}

void TitleScene::ParticleDraw() {}
