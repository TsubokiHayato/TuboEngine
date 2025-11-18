#include "StageClearState.h"
#include "StageScene.h"
#include <chrono>
#include <thread>

// クリア中は常時スロー（挙動のみ間引き）
namespace {
	// 挙動のスロー倍率（0.25 = 1/4 速度 = 実質15Hzでワールド更新）
	constexpr float kBehaviorSlowScale = 0.25f;

	// 実行状態
	bool gSlowActive = false;

	// 挙動スロー（ワールド更新の間引き）用
	std::chrono::steady_clock::time_point gBehaviorLast;
	double gAccumulatorSec = 0.0;
	double gWorldIntervalSec = 0.0; // = (1/60) / kBehaviorSlowScale

	inline bool ShouldUpdateWorld() {
		if (!gSlowActive) {
			return true;
		}
		using namespace std::chrono;
		const auto now = steady_clock::now();

		// 経過時間を蓄積し、一定間隔ごとにワールド更新を行う
		gAccumulatorSec += duration<double>(now - gBehaviorLast).count();
		gBehaviorLast = now;

		if (gAccumulatorSec >= gWorldIntervalSec) {
			gAccumulatorSec -= gWorldIntervalSec;
			return true; // このフレームはワールド更新を実行
		}
		return false; // このフレームはワールド更新をスキップ
	}
}

// StageClearState
void StageClearState::Enter(StageScene* scene) {
	// クリア中は常時スロー開始（Stateを抜けるまで維持）
	gSlowActive = true;
	gBehaviorLast = std::chrono::steady_clock::now();
	gAccumulatorSec = 0.0;
	// 通常 60fps 前提の更新間隔（例: 0.25倍速なら 1/60 / 0.25 ≒ 0.0667秒 = 15Hz）
	gWorldIntervalSec = (1.0 / 60.0) / static_cast<double>(kBehaviorSlowScale);

	restartSprite_ = std::make_unique<Sprite>();
	restartSprite_->Initialize("restart.png");
	restartSprite_->SetPosition({640.0f, 680.0f});
	restartSprite_->SetAnchorPoint({0.5f, 0.5f});
	restartSprite_->Update();
}

void StageClearState::Update(StageScene* scene) {
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		// ステージシーンをリスタートする処理
		scene->GetStageStateManager()->ChangeState(StageType::Ready, scene);
	}

	// UIは毎フレーム更新（点滅や入力反応のため）
	restartSprite_->Update();

	// アニメーションが終わったらシーン遷移
	// スペースキーで覆いを出すリクエスト

	
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		// アニメーション中は新たなアニメーションを開始しない
		if (scene->GetSceneChangeAnimation()->IsFinished()) {
			scene->GetSceneChangeAnimation()->SetPhase(SceneChangeAnimation::Phase::Appearing);
			scene->SetIsRequestSceneChange(true);
		}
	}

	// シーンチェンジアニメーション進行
	if (scene->GetIsRequestSceneChange() && scene->GetSceneChangeAnimation()->IsFinished()) {
		// ここでシーン遷移処理を呼ぶ
		SceneManager::GetInstance()->ChangeScene(SCENE::CLEAR);
		scene->SetIsRequestSceneChange(false); // フラグリセット
	}




	// 挙動自体のスロー：このフレームでワールドを更新するかを決める
	const bool doWorldUpdate = ShouldUpdateWorld();
	if (!doWorldUpdate) {
		return; // ワールド更新をスキップし、描画だけ行う
	}

	// 以降はワールド更新
	scene->GetFollowCamera()->Update();

	// スカイドーム
	scene->GetSkyDome()->SetCamera(scene->GetFollowCamera()->GetCamera());
	scene->GetSkyDome()->Update();

	// タイル
	std::vector<std::unique_ptr<Tile>>& tiles_ = scene->GetTiles();
	for (auto& tile : tiles_) {
		tile->SetCamera(scene->GetFollowCamera()->GetCamera());
		tile->Update();
	}

	// ブロック
	std::vector<std::unique_ptr<Block>>& blocks_ = scene->GetBlocks();
	for (auto& block : blocks_) {
		block->SetCamera(scene->GetFollowCamera()->GetCamera());
		block->Update();
	}

	// プレイヤー
	scene->GetPlayer()->Update();

	// 敵
	std::vector<std::unique_ptr<Enemy>>& enemies = scene->GetEnemies();
	for (auto& enemy : enemies) {
		enemy->SetCamera(scene->GetFollowCamera()->GetCamera());
		enemy->Update();
	}
}

void StageClearState::Exit(StageScene* scene) {
	// State を抜けたらスロー解除
	gSlowActive = false;
}

void StageClearState::Object3DDraw(StageScene* scene) {
	// 3Dオブジェクトの描画
	scene->GetSkyDome()->Draw();

	std::vector<std::unique_ptr<Tile>>& tiles_ = scene->GetTiles();
	for (auto& tile : tiles_) {
		tile->Draw();
	}

	std::vector<std::unique_ptr<Block>>& blocks_ = scene->GetBlocks();
	for (auto& block : blocks_) {
		block->Draw();
	}

	scene->GetPlayer()->Draw();

	std::vector<std::unique_ptr<Enemy>>& enemies = scene->GetEnemies();
	for (auto& enemy : enemies) {
		enemy->Draw();
	}
}

void StageClearState::SpriteDraw(StageScene* scene) { restartSprite_->Draw(); }

void StageClearState::ImGuiDraw(StageScene* scene) {}

void StageClearState::ParticleDraw(StageScene* scene) {}
