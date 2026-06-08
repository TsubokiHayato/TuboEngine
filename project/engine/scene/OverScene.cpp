#include "OverScene.h"
#include <algorithm>
#include <cmath>
#include"OffScreenRendering.h"
#include"TextManager.h"
#include "WinApp.h"
#include"Input.h"
#include"SceneManager.h"
#include "application/Stage/StageManager.h"

namespace {
	// Smoothstep
	float SmoothStep(float t){ return t*t*(3.0f-2.0f*t); }
	// EaseOutBounce (0..1)
	float EaseOutBounce(float t){
		const float n1 = 7.5625f;
		const float d1 = 2.75f;
		if (t < 1.0f / d1) {
			return n1 * t * t;
		} else if (t < 2.0f / d1) {
			t -= 1.5f / d1; return n1 * t * t + 0.75f;
		} else if (t < 2.5f / d1) {
			t -= 2.25f / d1; return n1 * t * t + 0.9375f;
		} else {
			t -= 2.625f / d1; return n1 * t * t + 0.984375f;
		}
	}
}

void OverScene::Initialize() {

	//ポストエフェクト設定
	OffScreenRendering::GetInstance()->SetVHSEffect(true);
	// カメラの初期化
	camera = std::make_unique<TuboEngine::Camera>();
	cameraTransform.rotate = {0.0f, 0.0f, 0.0f};      // カメラの回転を初期化
	cameraTransform.scale = {1.0f, 1.0f, 1.0f};       // カメラのスケールを初期化
	cameraTransform.translate = {0.0f, 0.0f, -15.0f}; // カメラの位置を初期化
	camera->SetTranslate(cameraTransform.translate);
	camera->setRotation(cameraTransform.rotate);
	camera->setScale(cameraTransform.scale);

	player = std::make_unique<Player>();
	player->Initialize();
	player->SetMovementLocked(true);
	player->SetCamera(camera.get());

	// アニメ開始位置へ
	player->SetPosition(startPos_);
	// 初期回転リセット
	player->SetRotation({0.0f, 0.0f, 0.0f});
	animTime_ = 0.0f;

	//TextManager（案内テキストは JSON、GAME OVER 見出しはコードで生成して演出する）
	TuboEngine::TextManager::GetInstance()->Initialize();
	if (!TuboEngine::TextManager::GetInstance()->LoadTextLayout("Resources/Text/GameOver.json")) {
		// JSON が無ければ案内テキストをコードで生成する
		if (TuboEngine::TextManager::GetInstance()->GetOrCreateFontSized(TuboEngine::TextManager::PresetFontNames::Best10, 48.0f)) {
			auto* textObj = TuboEngine::TextManager::GetInstance()->CreateText(
				TuboEngine::TextManager::PresetFontNames::Best10 + "_48",
				"Push SPACE to Return Stage",
				{ 640.0f, 550.0f },
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				1.0f
			);
			if (textObj) {
				textObj->SetHorizontalAlign(1); // 1 = Center
				textObj->SetVerticalAlign(1);   // 1 = Middle
			}
		}
	}

	// GAME OVER 見出し（演出対象）。ポインタを保持して Update で動かす。
	gameOverText_ = nullptr;
	goAnimTime_ = 0.0f;
	if (TuboEngine::TextManager::GetInstance()->GetOrCreateFontSized(TuboEngine::TextManager::PresetFontNames::Best10, 128.0f)) {
		gameOverText_ = TuboEngine::TextManager::GetInstance()->CreateText(
			TuboEngine::TextManager::PresetFontNames::Best10 + "_128",
			"GAME OVER",
			goStartPos_,                    // 画面外上から開始
			{ 1.0f, 0.15f, 0.15f, 0.0f },   // 赤・透明から
			goStartScale_                   // 大きめから着地でインパクト
		);
		if (gameOverText_) {
			gameOverText_->SetHorizontalAlign(1); // 1 = Center
			gameOverText_->SetVerticalAlign(1);   // 1 = Middle
		}
	}
}
void OverScene::Update() {

	// ポストエフェクト設定
	OffScreenRendering::GetInstance()->SetVHSEffect(true);

	// カメラ更新
	camera->SetTranslate(cameraTransform.translate);
	camera->setRotation(cameraTransform.rotate);
	camera->setScale(cameraTransform.scale);
	camera->Update();

	// デバッグカメラ（F2でON/OFF）。有効中はメインカメラを乗っ取る
	debugCamera_.Update(camera.get());

	// 落下＋バウンスをイージングで
	const float dt = 1.0f/60.0f;
	animTime_ += dt;
	float t = animDuration_ > 0.0f ? std::min(animTime_/animDuration_, 1.0f) : 1.0f;
	// Yはバウンス、Xはスムースに補間
	float y0 = startPos_.y;
	float y1 = endPos_.y; // 0
	float by = EaseOutBounce(t); // 0..1
	float y = y0 + (y1 - y0) * by;
	// Xは少し寄せる（開始X→終了X+xOffset_）
	float ex = endPos_.x + xOffset_;
	float x = startPos_.x + (ex - startPos_.x) * SmoothStep(t);
	TuboEngine::Math::Vector3 pos = {x, y, endPos_.z};
	player->SetPosition(pos);

	// 横倒れ（Z軸回りに倒す）: tに合わせて0→目標角
	float tiltT = SmoothStep(t);
	TuboEngine::Math::Vector3 rot = player->GetRotation();
	rot.z = -tiltSign_ * tiltTargetRad_ * tiltT;
	player->SetRotation(rot);

	player->Update();

	// ループ動作（任意）
	if (loopBounce_ && t >= 1.0f){
		animTime_ = 0.0f;
		player->SetPosition(startPos_);
		player->SetRotation({0.0f,0.0f,0.0f});
	}

	if (TuboEngine::Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		StageManager::SetShowRestartMessage(true);
		SceneManager::GetInstance()->ChangeScene(STAGE);
	}

	// GAME OVER 文字の演出：上から落下→バウンド着地→赤いネオン風フリッカー
	if (gameOverText_) {
		goAnimTime_ += dt;
		float gt = goDropDuration_ > 0.0f ? std::min(goAnimTime_ / goDropDuration_, 1.0f) : 1.0f;

		// Y はバウンスで落下、X は中央固定
		float by = EaseOutBounce(gt);
		float gy = goStartPos_.y + (goEndPos_.y - goStartPos_.y) * by;
		gameOverText_->SetPosition({ goEndPos_.x, gy });

		// スケールは大→等倍でインパクトを演出
		float gs = goStartScale_ + (goEndScale_ - goStartScale_) * SmoothStep(gt);
		gameOverText_->SetScale(gs);

		// フェードイン（最初の0.3秒で出現）
		float fadeIn = SmoothStep(std::min(goAnimTime_ / 0.3f, 1.0f));

		// 着地後は不気味なネオン風フリッカー
		float flicker = 1.0f;
		if (gt >= 1.0f) {
			float ft = goAnimTime_ - goDropDuration_;
			flicker = 0.72f + 0.28f * std::sin(ft * 17.0f) * std::sin(ft * 2.3f + 0.5f);
		}

		// 赤いゲームオーバーカラー（明滅で緑青成分を僅かに揺らす）
		float g = 0.10f + 0.12f * flicker;
		float b = 0.10f + 0.12f * flicker;
		gameOverText_->SetColor({ 1.0f, g, b, fadeIn * flicker });
	}


	// TextManager
	TuboEngine::TextManager::GetInstance()->UpdateAll();
}

void OverScene::Finalize() {
	// ポストエフェクト設定
	OffScreenRendering::GetInstance()->SetVHSEffect(false);

	// 次のシーンへ行く前に、テキストをすべてリセット（クリア）して持ち越さないようにする
	TuboEngine::TextManager::GetInstance()->ClearAllTexts();
	gameOverText_ = nullptr; // ClearAllTexts で破棄されるため参照を切る
}

void OverScene::Object3DDraw() { player->Draw(); }

void OverScene::SpriteDraw() {
	 // TextManager Draw
	TuboEngine::TextManager::GetInstance()->DrawAll();
}

void OverScene::ImGuiDraw() {

#ifdef USE_IMGUI
	TuboEngine::TextManager::GetInstance()->DrawImGui();


	ImGui::Begin("OVERScene");
	ImGui::Text("OVERScene");
	ImGui::Checkbox("Loop", &loopBounce_);
	ImGui::DragFloat("Duration", &animDuration_, 0.01f, 0.1f, 5.0f);
	ImGui::DragFloat3("StartPos", &startPos_.x, 0.05f);
	ImGui::DragFloat3("EndPos", &endPos_.x, 0.05f);
	ImGui::DragFloat("X Offset", &xOffset_, 0.01f, -5.0f, 5.0f);
	ImGui::DragFloat("TiltTargetRad", &tiltTargetRad_, 0.01f, 0.0f, 3.14f);
	ImGui::SliderInt("TiltSign (+Right/-Left)", &tiltSign_, -1, 1);
	ImGui::End();

	ImGui::Begin("Camera");
	ImGui::DragFloat3("Camera Translate", &cameraTransform.translate.x, 0.1f);
	ImGui::DragFloat3("Camera Rotate", &cameraTransform.rotate.x, 0.01f);
	ImGui::DragFloat3("Camera Scale", &cameraTransform.scale.x, 0.01f);
	ImGui::End();

	// デバッグカメラ操作UI
	debugCamera_.DrawImGui();
#endif // USE_IMGUI
}

void OverScene::ParticleDraw() {}
