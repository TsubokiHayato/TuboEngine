#include "TitleScene.h"
#include "StageScene.h" // 追加: isDemoModeフラグ操作のため
#include "ImGuiManager.h"
#include "Input.h"
#include "LineManager.h"
#include "SceneManager.h"
#include "TextureManager.h"
#include "numbers"
#include <algorithm> // std::clamp用
#include <cmath>     // 追加

void TitleScene::Initialize() {


	
    // カメラ（3D向けに調整）
	camera = std::make_unique<Camera>();
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);

	// Line 描画用にデフォルトカメラをセット（ここで一度登録しておく）
	LineManager::GetInstance()->SetDefaultCamera(camera.get());

	// タイトル用スカイドーム
	skyDome_ = std::make_unique<SkyDome>();
	// 少し大きめのスケールでタイトル用に初期化
	skyDome_->Initialize({0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f});
	skyDome_->SetCamera(camera.get());

	titleUI = std::make_unique<TitleUI>();
	titleUI->Initialize();

	// Scene change animation
	sceneChangeAnimation = std::make_unique<SceneChangeAnimation>(1280, 720, 80, 1.5f, "barrier.png");
	sceneChangeAnimation->Initialize();
	isRequestSceneChange = false;

	// プレイヤー初期化（タイトル用）
	player_ = std::make_unique<Player>();
	player_->Initialize();
	// タイトルでは自動操作させない
	player_->SetMovementLocked(true);
	// カメラをセット（描画用）
	player_->SetCamera(camera.get());
	// 初期は画面左外から入ってくる位置にセット（ワールド座標系前提）
	player_->SetPosition({ -6.0f, 0.0f, 0.0f }); // 左端スタート
	player_->SetScale({ 1.0f, 1.0f, 1.0f });
	player_->Update();

	playerIntroDone_ = false;
	playerIntroTimer_ = 0.0f;
	playerIdleTime_ = 0.0f;

	// 背景タイマー
	time_ = 0.0f;
	// デモタイマーリセット
	demoTimer_ = 0.0f;
}

void TitleScene::Update() {
	// 固定60FPS想定（必要なら dt を取る実装に変更）
	const float dt = 1.0f / 60.0f;
	time_ += dt;

	// スカイドーム回転
	if (skyDome_) {
		const float rotateSpeedYDegPerSec = 10.0f; // Y の回転速度
		const float rotateSpeedXDegPerSec = 5.0f;  // X の回転速度（お好みで）

		skyboxRotateY_ += rotateSpeedYDegPerSec * dt;
		skyboxRotateX_ += rotateSpeedXDegPerSec * dt;

		float rotY = skyboxRotateY_ * (std::numbers::pi_v<float> / 180.0f);
		float rotX = skyboxRotateX_ * (std::numbers::pi_v<float> / 180.0f);

		TuboEngine::Math::Vector3 rot = skyDome_->GetRotation();
		rot.y = rotY;
		rot.x = rotX; // ← X 軸も反映
		skyDome_->SetRotation(rot);

		// カメラ位置に追従させる
		skyDome_->SetPosition(camera->GetTranslate());
		skyDome_->Update();
	}

	// Player アニメーション: Intro -> Idle (呼吸・小さな振り向き・たまに hop)
	if (player_) {
		playerIntroTimer_ += (!playerIntroDone_) ? dt : 0.0f;

		TuboEngine::Math::Vector3 startPos = {-6.0f, 0.0f, 0.0f};
		TuboEngine::Math::Vector3 targetPos = {0.0f, 0.0f, 0.0f};

		TuboEngine::Math::Vector3 pos = player_->GetPosition();
		TuboEngine::Math::Vector3 rot = {};
		TuboEngine::Math::Vector3 scl = player_->GetScale();

		// Intro: 左からゆっくり歩いてくる
		if (!playerIntroDone_) {
			float t = std::clamp(playerIntroTimer_ / playerIntroDuration_, 0.0f, 1.0f);
			// イーズ（ややスムーズに）
			float ease = 1.0f - std::pow(1.0f - t, 3.0f);
			pos.x = startPos.x + (targetPos.x - startPos.x) * ease;
			// 少し前進（z方向）の演出
			pos.z = startPos.z + (targetPos.z - startPos.z) * ease;
			// ほんの少し回転して歩く感じ
			rot.y = 0.25f * std::sin(ease * 3.14f);
			// 歩き終わったら完了フラグ
			if (t >= 1.0f) {
				playerIntroDone_ = true;
				playerIdleTime_ = 0.0f;
			}
		}
		else {
			// Idle ループ: 呼吸 (上下のゆっくりな bob) + 小さな左右体の揺れ + ときどき hop
			playerIdleTime_ += dt;
			// 呼吸ボブ
			float breathe = 0.14f * std::sin(playerIdleTime_ * 1.6f); // y振幅
			// 小さな前後スウェイ
			float sway = 0.06f * std::sin(playerIdleTime_ * 0.8f);
			
			pos = targetPos;
			pos.y += breathe;
			pos.z += sway;

			// 体のそよぎ（Y軸回転でわずかに振り向く）
			rot.y = 0.12f * std::sin(playerIdleTime_ * 0.9f);

			// 呼吸に合わせて少しスケール伸縮（スクワッシュ/ストレッチ）
			float breathScale = 1.0f + 0.03f * std::sin(playerIdleTime_ * 1.6f);
			scl = { breathScale, 1.0f / breathScale, breathScale };
		}

		player_->SetPosition(pos);
		player_->SetRotation(rot);
		player_->SetScale(scl);
		// 描画用に内部更新
		player_->Update();
	}

	// カメラ更新
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);
	camera->Update();

	// LineManager にカメラを渡し、Line の内部更新を行う（カメラが未設定だとスクリーン変換されない）
	LineManager::GetInstance()->SetDefaultCamera(camera.get());
	LineManager::GetInstance()->Update();

	// シーンチェンジアニメーション更新
	sceneChangeAnimation->Update(dt);

	// UI更新
	titleUI->Update();

	// UIからのシーン遷移要求を受け取って、演出開始
	if (!isRequestSceneChange && titleUI && titleUI->GetrRequestSceneChange_()) {
		if (sceneChangeAnimation->IsFinished()) {
			// pending に UI の希望を保存してアニメ開始
			pendingNextSceneType_ = titleUI->GetNextSceneType();
			sceneChangeAnimation->SetPhase(SceneChangeAnimation::Phase::Appearing);
			isRequestSceneChange = true;
		}
	}

	// シーン遷移完了判定（UIの要求に応じて遷移）
	if (isRequestSceneChange && sceneChangeAnimation->IsFinished()) {
		int next = TITLE;
		// UI 主導の遷移があれば優先
		if (titleUI && titleUI->GetrRequestSceneChange_()) {
			switch (titleUI->GetNextSceneType()) {
			case SceneType::Select: next = STAGE; break;
			// Tutorialには行けないようにする
			case SceneType::Tutorial: next = TITLE; break;
			default: next = TITLE; break;
			}
			titleUI->ClearSceneChangeRequest();
		} else {
			// pendingNextSceneType_ を使う（デモなど）
			switch (pendingNextSceneType_) {
			case SceneType::Select: next = STAGE; break;
			// Tutorialには行けないようにする
			case SceneType::Tutorial: next = TITLE; break;
			default: next = TITLE; break;
			}
			// reset pending
			pendingNextSceneType_ = SceneType::Title;
		}
		// ChangeScene 実行
		SceneManager::GetInstance()->ChangeScene(next);
		isRequestSceneChange = false;
	}

	// 背景アニメ（時間のみで更新、Object3DDraw で参照）

	// --- Demo Timer Logic ---
	// 何か入力があればタイマーリセット
	auto* input = TuboEngine::Input::GetInstance();
	if (input->TriggerKey(DIK_SPACE) || input->TriggerKey(DIK_RETURN) ||
		input->TriggerKey(DIK_Z) || input->TriggerKey(DIK_X) ||
		input->IsTriggerMouse(0) || input->IsTriggerMouse(1) ||
		input->GetWheel() != 0) {
		demoTimer_ = 0.0f;
	} else {
		// 入力がなければ進める
		if (!isRequestSceneChange) { // シーン遷移中はカウントしない
			demoTimer_ += dt;
		}
	}

	// 時間経過でデモモードへ遷移
	if (demoTimer_ >= kDemoStartTime) {
		// アニメーション経由でデモに遷移する
		if (!isRequestSceneChange && sceneChangeAnimation && sceneChangeAnimation->IsFinished()) {
			pendingNextSceneType_ = SceneType::Select; // 実行時に STAGE へ
			// Set demo flag now so StageScene initializes in demo mode
			StageScene::isDemoMode = true;
			sceneChangeAnimation->SetPhase(SceneChangeAnimation::Phase::Appearing);
		 isRequestSceneChange = true;
		} else if (!sceneChangeAnimation) {
			// フォールバック
			StageScene::isDemoMode = true;
			SceneManager::GetInstance()->ChangeScene(STAGE);
		}
		// reset timer
		demoTimer_ = 0.0f;
	}
}

void TitleScene::Finalize() {}

void TitleScene::Object3DDraw() {

	// まずスカイドームを描画
	if (skyDome_) {
		skyDome_->Draw();
	}

	
	// プレイヤー本体の描画（3D）
	if (player_) player_->Draw();
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
	// カメラ
	ImGui::Begin("camera");
	ImGui::DragFloat3("Position", &cameraPosition.x, 0.01f);
	ImGui::DragFloat3("Rotation", &cameraRotation.x, 0.01f);
	ImGui::DragFloat3("Scale", &cameraScale.x, 0.01f);
	ImGui::End();

	if (sceneChangeAnimation) {
		sceneChangeAnimation->DrawImGui();
	}

	// プレイヤー調整（デバッグ用）
	if (player_) {
		ImGui::Begin("Player Title Anim");
		TuboEngine::Math::Vector3 playerPos_ = player_->GetPosition();
		ImGui::Text("Position: %.2f, %.2f, %.2f", playerPos_.x, playerPos_.y, playerPos_.z);
		ImGui::Text("IntroDone: %d", playerIntroDone_ ? 1 : 0);
		ImGui::End();
	}

	// スカイドーム調整用
	if (skyDome_) {
		skyDome_->DrawImGui("Title SkyDome");
	}
#endif // USE_IMGUI
}

void TitleScene::ParticleDraw() {}
