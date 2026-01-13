#include "TutorialState.h"

#include "StageScene.h"
#include "TextureManager.h"
#include "Input.h"
#include "Sprite.h"

namespace {
	template<typename Func>
	void ForEachMapChip(MapChipField* field, Func func) {
		if (!field) {
			return;
		}
		for (uint32_t y = 0; y < field->GetNumBlockVirtical(); ++y) {
			for (uint32_t x = 0; x < field->GetNumBlockHorizontal(); ++x) {
				func(x, y, field->GetMapChipTypeByIndex(x, y));
			}
		}
	}
}

void TutorialState::Enter(StageScene* scene) {
	elapsed_ = 0.0f;
	// 0: welcome, 1: move, 2: attack, 3: dash
	step_ = 0;
	built_ = false;

	InitializeTutorialSprites();
	BuildTutorialStage(scene);
}

void TutorialState::InitializeTutorialSprites() {
	// Use textures placed under the Tutorial folder.
	TextureManager::GetInstance()->LoadTexture("Tutorial/WelcomeTutorial.png");
	TextureManager::GetInstance()->LoadTexture("Tutorial/tutorial(Move).png");
	TextureManager::GetInstance()->LoadTexture("Tutorial/Tutorial(Attack).png");
	TextureManager::GetInstance()->LoadTexture("Tutorial/Tutorial(Dash).png");
	TextureManager::GetInstance()->LoadTexture("Tutorial/HowtoNext.png");
	TextureManager::GetInstance()->LoadTexture("Tutorial/TitleBack.png");

	// Layout (1280x720)
	const float leftX = 60.0f;
	const float baseY = 70.0f;

	// すべて同じ位置に表示（ページ切り替えで差し替え）
	const Vector2 pagePos{leftX, baseY};

	// Welcome
	tutorialHeaderSprite_ = std::make_unique<Sprite>();
	tutorialHeaderSprite_->Initialize("Tutorial/WelcomeTutorial.png");
	tutorialHeaderSprite_->SetAnchorPoint({0.0f, 0.0f});
	tutorialHeaderSprite_->SetGetIsAdjustTextureSize(true);
	tutorialHeaderSprite_->SetPosition(pagePos);
	tutorialHeaderSprite_->Update();

	// Move
	tutorialMoveSprite_ = std::make_unique<Sprite>();
	tutorialMoveSprite_->Initialize("Tutorial/tutorial(Move).png");
	tutorialMoveSprite_->SetAnchorPoint({0.0f, 0.0f});
	tutorialMoveSprite_->SetGetIsAdjustTextureSize(true);
	tutorialMoveSprite_->SetPosition(pagePos);
	tutorialMoveSprite_->Update();

	// Attack
	tutorialAttackSprite_ = std::make_unique<Sprite>();
	tutorialAttackSprite_->Initialize("Tutorial/Tutorial(Attack).png");
	tutorialAttackSprite_->SetAnchorPoint({0.0f, 0.0f});
	tutorialAttackSprite_->SetGetIsAdjustTextureSize(true);
	tutorialAttackSprite_->SetPosition(pagePos);
	tutorialAttackSprite_->Update();

	// Dash
	tutorialDashSprite_ = std::make_unique<Sprite>();
	tutorialDashSprite_->Initialize("Tutorial/Tutorial(Dash).png");
	tutorialDashSprite_->SetAnchorPoint({0.0f, 0.0f});
	tutorialDashSprite_->SetGetIsAdjustTextureSize(true);
	tutorialDashSprite_->SetPosition(pagePos);
	tutorialDashSprite_->Update();

	// HowtoNext: small
	howtoNextSprite_ = std::make_unique<Sprite>();
	howtoNextSprite_->Initialize("Tutorial/HowtoNext.png");
	howtoNextSprite_->SetAnchorPoint({0.0f, 0.0f});
	howtoNextSprite_->SetGetIsAdjustTextureSize(true);
	howtoNextSprite_->SetSize({180.0f, 30.0f});
	howtoNextSprite_->SetPosition({leftX, baseY + 60.0f});
	howtoNextSprite_->Update();

	// TitleBack: bottom-left (small)
	titleBackSprite_ = std::make_unique<Sprite>();
	titleBackSprite_->Initialize("Tutorial/TitleBack.png");
	titleBackSprite_->SetAnchorPoint({0.0f, 1.0f});
	titleBackSprite_->SetGetIsAdjustTextureSize(true);
	titleBackSprite_->SetSize({180.0f, 30.0f});
	titleBackSprite_->SetPosition({20.0f, 692.0f});
	titleBackSprite_->Update();
}

void TutorialState::BuildTutorialStage(StageScene* scene) {
	if (!scene) {
		return;
	}

	if (!scene->GetPlayer() || !scene->GetMapChipField() || !scene->GetFollowCamera()) {
		return;
	}

	scene->GetBlocks().clear();
	scene->GetEnemies().clear();
	scene->GetStageInstances().clear();

	scene->GetMapChipField()->ResetMapChipData();
	scene->GetMapChipField()->LoadMapChipCsv(tutorialCsvPath_);

	int playerMapX = -1;
	int playerMapY = -1;
	ForEachMapChip(scene->GetMapChipField(), [&](uint32_t x, uint32_t y, MapChipType type) {
		if (type == MapChipType::Player) {
			playerMapX = static_cast<int>(x);
			playerMapY = static_cast<int>(y);
		}
	});
	if (playerMapX < 0 || playerMapY < 0) {
		playerMapX = 0;
		playerMapY = 0;
	}
	Vector3 playerStartPos = scene->GetMapChipField()->GetMapChipPositionByIndex(
		static_cast<uint32_t>(playerMapX), static_cast<uint32_t>(playerMapY));

	scene->GetPlayer()->Initialize();
	scene->GetPlayer()->SetMapChipField(scene->GetMapChipField());
	scene->GetPlayer()->SetIsDontMove(false);
	scene->GetPlayer()->SetPosition(playerStartPos);

	// カメラ初期化
	scene->GetFollowCamera()->Initialize(scene->GetPlayer(), Vector3{0.0f, 0.0f, -70.0f}, 0.25f);
	scene->GetFollowCamera()->Update();
	scene->GetPlayer()->SetCamera(scene->GetFollowCamera()->GetCamera());
	scene->GetPlayer()->Update();

	// タイル初期化（ステージ左上基準）
	if (!scene->GetTile()) {
		scene->GetTile() = std::make_unique<Tile>();
	}
	{
		Vector3 tilePos = scene->GetMapChipField()->GetMapChipPositionByIndex(0, 0);
		tilePos.z = -1.0f;
		scene->GetTile()->Initialize(tilePos, {1.0f, 1.0f, 1.0f}, "tile/tile30x30.obj");
		scene->GetTile()->SetCamera(scene->GetFollowCamera()->GetCamera());
		scene->GetTile()->Update();
	}

	ForEachMapChip(scene->GetMapChipField(), [&](uint32_t x, uint32_t y, MapChipType type) {
		Vector3 pos = scene->GetMapChipField()->GetMapChipPositionByIndex(x, y);
		if (type == MapChipType::kBlock) {
			auto block = std::make_unique<Block>();
			block->Initialize(pos);
			block->SetCamera(scene->GetFollowCamera()->GetCamera());
			block->Update();
			scene->GetBlocks().push_back(std::move(block));
		} else if (type == MapChipType::Enemy || type == MapChipType::EnemyRush) {
			auto enemy = std::make_unique<RushEnemy>();
			enemy->Initialize();
			enemy->SetCamera(scene->GetFollowCamera()->GetCamera());
			enemy->SetPlayer(scene->GetPlayer());
			enemy->SetPosition(pos);
			enemy->SetMapChipField(scene->GetMapChipField());
			enemy->Update();
			scene->GetEnemies().push_back(std::move(enemy));
		} else if (type == MapChipType::EnemyShoot) {
			auto enemy = std::make_unique<Enemy>();
			enemy->Initialize();
			enemy->SetCamera(scene->GetFollowCamera()->GetCamera());
			enemy->SetPlayer(scene->GetPlayer());
			enemy->SetPosition(pos);
			enemy->SetMapChipField(scene->GetMapChipField());
			enemy->Update();
			scene->GetEnemies().push_back(std::move(enemy));
		}
	});

	if (scene->GetSkyDome()) {
		scene->GetSkyDome()->Initialize();
		scene->GetSkyDome()->SetCamera(scene->GetFollowCamera()->GetCamera());
		scene->GetSkyDome()->Update();
	}

	built_ = true;
}

void TutorialState::Update(StageScene* scene) {
	elapsed_ += 1.0f / 60.0f;

	if (!scene) {
		return;
	}
	if (!built_) {
		BuildTutorialStage(scene);
		if (!built_) {
			return;
		}
	}

	constexpr int kMaxStep = 3;

	// Page control
	// Enter : next page
	// Backspace : previous page
	if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
		if (step_ < kMaxStep) {
			step_++;
		}
	}
	if (Input::GetInstance()->TriggerKey(DIK_BACK)) {
		if (step_ > 0) {
			step_--;
		}
	}

	// ESCでタイトルへ戻る
	if (Input::GetInstance()->TriggerKey(DIK_ESCAPE)) {
		scene->SetSceneNo(TITLE);
		return;
	}

	if (!scene->GetPlayer() || !scene->GetMapChipField() || !scene->GetFollowCamera()) {
		return;
	}

	for (auto& block : scene->GetBlocks()) {
		if (!block) continue;
		block->SetCamera(scene->GetFollowCamera()->GetCamera());
		block->Update();
	}

	scene->GetPlayer()->SetCamera(scene->GetFollowCamera()->GetCamera());
	scene->GetPlayer()->SetIsDontMove(false);
	scene->GetPlayer()->SetMapChipField(scene->GetMapChipField());
	scene->GetPlayer()->Update();

	for (auto& enemy : scene->GetEnemies()) {
		if (!enemy) continue;
		enemy->SetCamera(scene->GetFollowCamera()->GetCamera());
		enemy->SetPlayer(scene->GetPlayer());
		enemy->SetMapChipField(scene->GetMapChipField());
		enemy->Update();
	}

	if (auto& tile = scene->GetTile()) {
		tile->SetCamera(scene->GetFollowCamera()->GetCamera());
		tile->Update();
	}

	scene->GetFollowCamera()->Update();

	if (scene->GetSkyDome()) {
		scene->GetSkyDome()->Update();
	}

	if (!scene->GetPlayer()->GetIsAllive()) {
		scene->GetStageStateManager()->ChangeState(StageType::GameOver, scene);
		return;
	}

	// UI update (only what is currently shown)
	if (step_ < kMaxStep) {
		if (howtoNextSprite_) howtoNextSprite_->Update();
	}
	if (titleBackSprite_) titleBackSprite_->Update();

	switch (step_) {
	case 0:
		if (tutorialHeaderSprite_) tutorialHeaderSprite_->Update();
		break;
	case 1:
		if (tutorialMoveSprite_) tutorialMoveSprite_->Update();
		break;
	case 2:
		if (tutorialAttackSprite_) tutorialAttackSprite_->Update();
		break;
	case 3:
		if (tutorialDashSprite_) tutorialDashSprite_->Update();
		break;
	default:
		break;
	}
}

void TutorialState::Exit(StageScene* /*scene*/) {
	// Release sprites explicitly (optional)
	tutorialHeaderSprite_.reset();
	tutorialMoveSprite_.reset();
	tutorialAttackSprite_.reset();
	tutorialDashSprite_.reset();
	howtoNextSprite_.reset();
	titleBackSprite_.reset();
}

void TutorialState::Object3DDraw(StageScene* scene) {
	if (!scene) {
		return;
	}

	if (scene->GetSkyDome()) {
		scene->GetSkyDome()->Draw();
	}

	if (auto& tile = scene->GetTile()) {
		tile->Draw();
	}

	for (auto& block : scene->GetBlocks()) {
		if (!block) continue;
		block->Draw();
	}

	if (scene->GetPlayer()) {
		scene->GetPlayer()->Draw();
	}

	for (auto& enemy : scene->GetEnemies()) {
		if (!enemy) continue;
		enemy->Draw();
	}
}

void TutorialState::SpriteDraw(StageScene* scene) {
	if (!scene || !scene->GetPlayer()) {
		return;
	}

	constexpr int kMaxStep = 3;

	// UI (paged)
	switch (step_) {
	case 0:
		if (tutorialHeaderSprite_) tutorialHeaderSprite_->Draw();
		break;
	case 1:
		if (tutorialMoveSprite_) tutorialMoveSprite_->Draw();
		break;
	case 2:
		if (tutorialAttackSprite_) tutorialAttackSprite_->Draw();
		break;
	case 3:
		if (tutorialDashSprite_) tutorialDashSprite_->Draw();
		break;
	default:
		break;
	}

	// Has next page (hide on last)
	if (step_ < kMaxStep) {
		if (howtoNextSprite_) howtoNextSprite_->Draw();
	}
	if (titleBackSprite_) titleBackSprite_->Draw();

	// Reticle
	scene->GetPlayer()->ReticleDraw();
}

void TutorialState::ImGuiDraw(StageScene* /*scene*/) {
}

void TutorialState::ParticleDraw(StageScene* scene) {
	if (!scene) {
		return;
	}
	for (auto& enemy : scene->GetEnemies()) {
		if (!enemy) continue;
		enemy->ParticleDraw();
	}
}
