#include "StageReadyState.h"
#include "StageScene.h"
#include "StageType.h"

void StageReadyState::Enter(StageScene* scene) {

	///---------------------------------------------------------
	/// 各キャラクターの初期化やマップチップの読み込みを行う
	///---------------------------------------------------------

	// マップチップフィールドの生成・初期化
	scene->GetMapChipField()->LoadMapChipCsv(scene->GetMapChipCsvFilePath());

	// 追従カメラの生成・初期化
	scene->GetFollowCamera()->Initialize(scene->GetPlayer(), Vector3(0.0f, 0.0f, 40.0f), 0.2f);
	scene->GetFollowCamera()->Update();

	// ブロック生成（マップチップ対応）
	scene->GetBlocks().clear();
	for (uint32_t y = 0; y < scene->GetMapChipField()->GetNumBlockVirtical(); ++y) {
		for (uint32_t x = 0; x < scene->GetMapChipField()->GetNumBlockHorizontal(); ++x) {
			if (scene->GetMapChipField()->GetMapChipTypeByIndex(x, y) == MapChipType::kBlock) {
				auto block = std::make_unique<Block>();
				block->Initialize(scene->GetMapChipField()->GetMapChipPositionByIndex(x, y));
				block->SetCamera(scene->GetFollowCamera()->GetCamera());
				block->Update();
				scene->GetBlocks().push_back(std::move(block));
			}
		}
	}

	// プレイヤー

	scene->GetPlayer()->Initialize();

	// プレイヤーの初期位置をマップチップから取得
	for (uint32_t y = 0; y < scene->GetMapChipField()->GetNumBlockVirtical(); ++y) {
		for (uint32_t x = 0; x < scene->GetMapChipField()->GetNumBlockHorizontal(); ++x) {
			if (scene->GetMapChipField()->GetMapChipTypeByIndex(x, y) == MapChipType::Player) {
				Vector3 pos = scene->GetMapChipField()->GetMapChipPositionByIndex(x, y);
				scene->GetPlayer()->SetPosition(scene->GetMapChipField()->GetMapChipPositionByIndex(x, y));
			}
		}
	}
	// プレイヤーにカメラをセット
	scene->GetPlayer()->SetCamera(scene->GetFollowCamera()->GetCamera());
	scene->GetPlayer()->SetMapChipField(scene->GetMapChipField());
	scene->GetPlayer()->Update();

	// プレイヤー生成後
	// player_->SetMapChipField(mapChipField_.get());
	// Enemyリスト
	scene->GetEnemies().clear();

	for (uint32_t y = 0; y < scene->GetMapChipField()->GetNumBlockVirtical(); ++y) {
		for (uint32_t x = 0; x < scene->GetMapChipField()->GetNumBlockHorizontal(); ++x) {
			if (scene->GetMapChipField()->GetMapChipTypeByIndex(x, y) == MapChipType::Enemy) {
				auto enemy = std::make_unique<Enemy>();
				enemy->Initialize();
				enemy->SetCamera(scene->GetFollowCamera()->GetCamera());
				enemy->SetPlayer(scene->GetPlayer());
				// enemy->SetMapChipField(scene->GetMapChipField());
				enemy->SetPosition(scene->GetMapChipField()->GetMapChipPositionByIndex(x, y));
				enemy->Update();
				scene->GetEnemies().push_back(std::move(enemy));
			}
		}
	}
}
void StageReadyState::Update(StageScene* scene) {

	// ステージ開始前の準備が完了したら、次の状態に遷移
	scene->GetStageStateManager()->ChangeState(StageType::Playing, scene);
}
void StageReadyState::Exit(StageScene* scene) {}

void StageReadyState::Object3DDraw(StageScene* scene) {}

void StageReadyState::SpriteDraw(StageScene* scene) {}

void StageReadyState::ImGuiDraw(StageScene* scene) {}

void StageReadyState::ParticleDraw(StageScene* scene) {}
