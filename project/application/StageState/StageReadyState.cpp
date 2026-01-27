#include "StageReadyState.h"
#include "LineManager.h"
#include "StageScene.h"
#include "StageType.h"
#include "TextureManager.h"
#include "WinApp.h"
#include "Sprite.h"

#undef max
#include <algorithm>

namespace {
	constexpr float kDeltaSec = 1.0f / 60.0f;
}

// 汎用: マップチップを走査してコールバック（StageScene版）
template<typename Func> static void ForEachMapChip(StageScene* scene, Func func) {
	auto* field = scene ? scene->GetMapChipField() : nullptr;
	if (!field) {
		return;
	}
	for (uint32_t y = 0; y < field->GetNumBlockVirtical(); ++y) {
		for (uint32_t x = 0; x < field->GetNumBlockHorizontal(); ++x) {
			func(x, y, field->GetMapChipTypeByIndex(x, y));
		}
	}
}

// 追加: 指定MapChipFieldを走査してコールバック（プレビュー用）
template<typename Func> static void ForEachMapChipField(MapChipField* field, Func func) {
	if (!field) {
		return;
	}
	for (uint32_t y = 0; y < field->GetNumBlockVirtical(); ++y) {
		for (uint32_t x = 0; x < field->GetNumBlockHorizontal(); ++x) {
			func(x, y, field->GetMapChipTypeByIndex(x, y));
		}
	}
}

void StageReadyState::Enter(StageScene* scene) {
	// Stage0（実プレイ）のCSVをロード
	scene->GetMapChipField()->LoadMapChipCsv(scene->GetMapChipCsvFilePath());

	// プレイヤー座標（Stage0）
	int playerMapX = -1, playerMapY = -1;
	ForEachMapChip(scene, [&](uint32_t x, uint32_t y, MapChipType type) {
		if (type == MapChipType::Player) {
			playerMapX = static_cast<int>(x);
			playerMapY = static_cast<int>(y);
		}
	});
	if (playerMapX < 0 || playerMapY < 0) {
		playerMapX = 0;
		playerMapY = 0;
	}
	Vector3 playerPos = scene->GetMapChipField()->GetMapChipPositionByIndex(playerMapX, playerMapY);

	// プレイヤー初期化（Stage0のみ）
	scene->GetPlayer()->Initialize();
	scene->GetPlayer()->SetMapChipField(scene->GetMapChipField());
	scene->GetPlayer()->SetIsDontMove(false);
	scene->GetPlayer()->SetPosition(playerPos);

	// フォローカメラ初期化（プレビュー生成にも使う）
	scene->GetFollowCamera()->Initialize(scene->GetPlayer(), Vector3{0.0f, 0.0f, -70.0f}, 0.08f);
	// ズーム制限設定
	scene->GetFollowCamera()->SetZoomLimits(0.1f, 1.0f);
	// 開始時は近い(0.1)状態に固定してからカメラ位置を確定
	scene->GetFollowCamera()->SetZoom(0.1f);
	// 初回だけは補間せずスナップして「追従している状態」から開始したい
	scene->GetFollowCamera()->SnapToTarget();
	// 開始時カメラ演出: Zoomをイージングで変化（0.1 -> 1.0）
	scene->GetFollowCamera()->StartIntroZoom(0.1f, 1.0f, 0.8f);
	Camera* cam = scene->GetFollowCamera()->GetCamera();
	LineManager::GetInstance()->SetDefaultCamera(cam);
	scene->GetPlayer()->SetCamera(cam);
	scene->GetPlayer()->Update();

	// ブロック・エネミーの生成（Stage0）
	auto& blocks = scene->GetBlocks();
	auto& enemies = scene->GetEnemies();
	blocks.clear();
	enemies.clear();

	ForEachMapChip(scene, [&](uint32_t x, uint32_t y, MapChipType type) {
		Vector3 pos = scene->GetMapChipField()->GetMapChipPositionByIndex(x, y);

		if (type == MapChipType::kBlock) {
			auto block = std::make_unique<Block>();
			block->Initialize(pos);
			block->SetCamera(cam);
			block->Update();
			blocks.push_back(std::move(block));
		} else if (type == MapChipType::Enemy || type == MapChipType::EnemyRush) {
			auto enemy = std::make_unique<RushEnemy>();
			enemy->Initialize();
			enemy->SetCamera(cam);
			enemy->SetPlayer(scene->GetPlayer());
			enemy->SetPosition(pos);
			enemy->Update();
			enemies.push_back(std::move(enemy));
		} else if (type == MapChipType::EnemyShoot) {
			auto enemy = std::make_unique<Enemy>();
			enemy->Initialize();
			enemy->SetCamera(cam);
			enemy->SetPlayer(scene->GetPlayer());
			enemy->SetPosition(pos);
			enemy->Update();
			enemies.push_back(std::move(enemy));
		}
	});

	// タイル（Stage0）
	Vector3 tilePos = scene->GetMapChipField()->GetMapChipPositionByIndex(0, 0);
	tilePos.z = -1.0f;
	scene->GetTile()->Initialize(tilePos, {1.0f, 1.0f, 1.0f}, "tile/tile30x30.obj");
	scene->GetTile()->SetCamera(cam);
	scene->GetTile()->Update();

	// SkyDome
	scene->GetSkyDome()->Initialize();
	scene->GetSkyDome()->SetCamera(cam);
	scene->GetSkyDome()->Update();

	// タイマー初期化
	prevTime_ = std::chrono::steady_clock::now();

	// --- Ready/Start 演出用スプライト ---
	phase_ = Phase::Ready;
	phaseTimer_ = 0.0f;

	const float screenW = static_cast<float>(WinApp::GetInstance()->GetClientWidth());
	const float screenH = static_cast<float>(WinApp::GetInstance()->GetClientHeight());
	const Vector2 center = { screenW * 0.5f, screenH * 0.5f };

	// テクスチャはプロジェクト側の配置に合わせて差し替えてください
	const std::string readyTex = "ready.png";
	const std::string startTex = "start.png";
	TextureManager::GetInstance()->LoadTexture(readyTex);
	TextureManager::GetInstance()->LoadTexture(startTex);

	readySprite_ = std::make_unique<Sprite>();
	readySprite_->Initialize(readyTex);
	readySprite_->SetAnchorPoint({ 0.5f, 0.5f });
	readySprite_->SetGetIsAdjustTextureSize(true);
	readySprite_->SetPosition(center);
	readySprite_->SetColor({ 1,1,1,1 });
	readySprite_->Update();

	startSprite_ = std::make_unique<Sprite>();
	startSprite_->Initialize(startTex);
	startSprite_->SetAnchorPoint({ 0.5f, 0.5f });
	startSprite_->SetGetIsAdjustTextureSize(true);
	startSprite_->SetPosition(center);
	startSprite_->SetColor({ 1,1,1,0 }); // Startは最初非表示
	startSprite_->Update();
}

void StageReadyState::Update(StageScene* scene) {
	using namespace std::chrono;
	auto now = steady_clock::now();
	float deltaTime = duration_cast<duration<float>>(now - prevTime_).count();
	(void)deltaTime;
	prevTime_ = now;

	// 先にプレイヤーを更新してからカメラを更新する（追従対象が最新になる）
	if (scene && scene->GetPlayer()) {
		scene->GetPlayer()->SetCamera(scene->GetFollowCamera()->GetCamera());
		scene->GetPlayer()->Update();
	}

	scene->GetFollowCamera()->Update();
	LineManager::GetInstance()->SetDefaultCamera(scene->GetFollowCamera()->GetCamera());

	for (auto& block : scene->GetBlocks()) {
		block->SetCamera(scene->GetFollowCamera()->GetCamera());
		block->Update();
	}
	for (auto& enemy : scene->GetEnemies()) {
		enemy->SetCamera(scene->GetFollowCamera()->GetCamera());
		enemy->SetPlayer(scene->GetPlayer());
		enemy->Update();
	}
	scene->GetTile()->SetCamera(scene->GetFollowCamera()->GetCamera());
	scene->GetTile()->Update();

	scene->GetSkyDome()->SetCamera(scene->GetFollowCamera()->GetCamera());
	scene->GetSkyDome()->Update();

	// --- Ready/Start 演出フェーズ進行 ---
	phaseTimer_ += kDeltaSec;

	auto SetSpriteAlpha = [](Sprite* spr, float a) {
		if (!spr) return;
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

	if (readySprite_) readySprite_->Update();
	if (startSprite_) startSprite_->Update();

	// 演出が終わったらPlayingへ
	if (phase_ == Phase::Done && !scene->GetFollowCamera()->IsIntroZoomPlaying()) {
		scene->GetStageStateManager()->ChangeState(StageType::Playing, scene);
	}
}

void StageReadyState::Exit(StageScene* scene) {}

void StageReadyState::Object3DDraw(StageScene* scene) {
	// Stage0
	for (auto& block : scene->GetBlocks()) {
		block->Draw();
	}
	scene->GetPlayer()->Draw();
	for (auto& enemy : scene->GetEnemies()) {
		enemy->Draw();
	}
	scene->GetTile()->Draw();
	scene->GetSkyDome()->Draw();
}

void StageReadyState::SpriteDraw(StageScene* scene) {
	(void)scene;
	if (readySprite_) readySprite_->Draw();
	if (startSprite_) startSprite_->Draw();
}

void StageReadyState::ImGuiDraw(StageScene* scene) { scene->GetFollowCamera()->DrawImGui(); }

void StageReadyState::ParticleDraw(StageScene* scene) { (void)scene; }
