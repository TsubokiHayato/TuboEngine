#include "StageReadyState.h"
#include "LineManager.h"
#include "StageScene.h"
#include "StageState/StageType.h"
#include "TextureManager.h"
#include "WinApp.h"

#undef max
#include <algorithm>

namespace {
constexpr float kDeltaSec = 1.0f / 60.0f;
}

void StageReadyState::Enter(StageScene* scene) {
	if (!scene)
		return;

	auto* stageMgr = scene->GetStageManager();
	auto* followCam = scene->GetFollowCamera();
	auto* player = scene->GetPlayer();

	// プレイヤー初期化
	if (player) {
		player->Initialize();
	}

	// カメラ初期化
	if (followCam && player) {
		followCam->Initialize(player, TuboEngine::Math::Vector3{0.0f, 0.0f, -70.0f}, 0.08f);
		followCam->SetZoomLimits(0.1f, 1.0f);
		followCam->SetZoom(0.1f);
		followCam->SnapToTarget();
		followCam->StartIntroZoom(0.1f, 1.0f, 0.8f);
		TuboEngine::Camera* cam = followCam->GetCamera();
		LineManager::GetInstance()->SetDefaultCamera(cam);
		player->SetCamera(cam);

		// SkyDome 初期化
		if (auto* sky = scene->GetSkyDome().get()) {
			sky->Initialize();
			sky->SetCamera(cam);
			sky->Update();
		}
	}

	// ★ カメラ初期化後に StageManager のメタレイアウトをロード
	if (stageMgr && followCam && player && scene->GetDrawPreviewStages()) {
		stageMgr->LoadMetaLayout("Resources/Stage/Stage.csv", player, followCam);

		// LoadMetaLayout 後に、StageManager 側の開始位置 & Field に合わせて再設定
		TuboEngine::Math::Vector3 startPos = stageMgr->GetPlayerStartPosition();
		MapChipField* startField = stageMgr->GetPlayerStartField();
		if (startField) {
			player->SetMapChipField(startField);
		}
		player->SetPosition(startPos);
		// カメラにもターゲット変更を反映
		followCam->SetTarget(player);
	}

	// タイマー初期化
	prevTime_ = std::chrono::steady_clock::now();

	// Ready/Start スプライト初期化（従来通り）
	phase_ = Phase::Ready;
	phaseTimer_ = 0.0f;

	const float screenW = static_cast<float>(TuboEngine::WinApp::GetInstance()->GetClientWidth());
	const float screenH = static_cast<float>(TuboEngine::WinApp::GetInstance()->GetClientHeight());
	const TuboEngine::Math::Vector2 center = {screenW * 0.5f, screenH * 0.5f};

	const std::string readyTex = "ready.png";
	const std::string startTex = "start.png";
	TuboEngine::TextureManager::GetInstance()->LoadTexture(readyTex);
	TuboEngine::TextureManager::GetInstance()->LoadTexture(startTex);

	readySprite_ = std::make_unique<TuboEngine::Sprite>();
	readySprite_->Initialize(readyTex);
	readySprite_->SetAnchorPoint({0.5f, 0.5f});
	readySprite_->SetGetIsAdjustTextureSize(true);
	readySprite_->SetPosition(center);
	readySprite_->SetColor({1, 1, 1, 1});
	readySprite_->Update();

	startSprite_ = std::make_unique<TuboEngine::Sprite>();
	startSprite_->Initialize(startTex);
	startSprite_->SetAnchorPoint({0.5f, 0.5f});
	startSprite_->SetGetIsAdjustTextureSize(true);
	startSprite_->SetPosition(center);
	startSprite_->SetColor({1, 1, 1, 0});
	startSprite_->Update();
}

void StageReadyState::Update(StageScene* scene) {
	using namespace std::chrono;
	auto now = steady_clock::now();
	float deltaTime = duration_cast<duration<float>>(now - prevTime_).count();
	(void)deltaTime;
	prevTime_ = now;

	if (!scene)
		return;

	auto* player = scene->GetPlayer();
	auto* followCam = scene->GetFollowCamera();
	auto* stageMgr = scene->GetStageManager();

	// 先にプレイヤーを更新してからカメラを更新
	if (player && followCam) {
		player->SetMovementLocked(true);
		player->SetCamera(followCam->GetCamera());
		player->Update();

		followCam->Update();
		LineManager::GetInstance()->SetDefaultCamera(followCam->GetCamera());
	}

	// StageManager 管理下のステージオブジェクトを更新
	if (stageMgr && followCam) {
		stageMgr->Update(player, followCam);
	}

	// SkyDome
	if (auto* sky = scene->GetSkyDome().get()) {
		sky->SetCamera(followCam->GetCamera());
		sky->Update();
	}

	// --- Ready/Start 演出フェーズ進行 ---
	phaseTimer_ += kDeltaSec;

	auto SetSpriteAlpha = [](TuboEngine::Sprite* spr, float a) {
		if (!spr)
			return;
		Vector4 c = spr->GetColor();
		c.w = std::clamp(a, 0.0f, 1.0f);
		spr->SetColor(c);
	};

	if (phase_ == Phase::Ready) {
		SetSpriteAlpha(readySprite_.get(), 1.0f);
		SetSpriteAlpha(startSprite_.get(), 0.0f);
		if (phaseTimer_ >= readyDuration_) {
			phase_ = Phase::Start;
			phaseTimer_ = 0.0f;
		}
	} else if (phase_ == Phase::Start) {
		SetSpriteAlpha(readySprite_.get(), 0.0f);
		SetSpriteAlpha(startSprite_.get(), 1.0f);
		if (phaseTimer_ >= startDuration_) {
			phase_ = Phase::Done;
		}
	}

	if (readySprite_)
		readySprite_->Update();
	if (startSprite_)
		startSprite_->Update();

	// 演出が終わったらPlayingへ
	if (phase_ == Phase::Done && followCam && !followCam->IsIntroZoomPlaying()) {
		if (auto* mgr = scene->GetStageStateManager()) {
			mgr->ChangeState(StageType::Playing, scene);
		}
	}
}

void StageReadyState::Exit(StageScene* scene) { (void)scene; }

void StageReadyState::Object3DDraw(StageScene* scene) {
	if (!scene)
		return;
	auto* stageMgr = scene->GetStageManager();
	if (stageMgr) {
		stageMgr->Draw3D();
	}
	if (auto* sky = scene->GetSkyDome().get()) {
		sky->Draw();
	}

	Player* player = scene->GetPlayer();
	player->Draw();
}

void StageReadyState::SpriteDraw(StageScene* scene) {
	(void)scene;
	if (readySprite_)
		readySprite_->Draw();
	if (startSprite_)
		startSprite_->Draw();
}

void StageReadyState::ImGuiDraw(StageScene* scene) {
	if (scene && scene->GetFollowCamera())
		scene->GetFollowCamera()->DrawImGui();
}

void StageReadyState::ParticleDraw(StageScene* scene) { (void)scene; }
