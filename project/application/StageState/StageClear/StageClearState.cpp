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
} // namespace

// StageClearState
void StageClearState::Enter(StageScene* scene) {
	// クリア中は常時スロー開始（Stateを抜けるまで維持）
	gSlowActive = true;
	gBehaviorLast = std::chrono::steady_clock::now();
	gAccumulatorSec = 0.0;
	// 通常 60fps 前提の更新間隔（例: 0.25倍速なら 1/60 / 0.25 ≒ 0.0667秒 = 15Hz）
	gWorldIntervalSec = (1.0 / 60.0) / static_cast<double>(kBehaviorSlowScale);

	// 自動シーン遷移タイマー初期化
	started_ = true;
	timer_ = 0.0f;
	lastFrameTime_ = std::chrono::steady_clock::now();
	(void)scene; // 現状 Enter では scene を直接は使っていないため警告抑制
}

void StageClearState::Update(StageScene* scene) {
	if (!scene) return;

	// 経過時間計測（スローに関係なく実時間で進行）
	{
		const auto now = std::chrono::steady_clock::now();
		timer_ += static_cast<float>(std::chrono::duration<double>(now - lastFrameTime_).count());
		lastFrameTime_ = now;
	}

	// 一定時間後にシーンチェンジ演出開始（以前の「スペース押したら」部分を時間経過に置き換え）
	if (!scene->GetIsRequestSceneChange() && started_ && timer_ >= durationSec_) {
		// アニメーションが完全にアイドル状態なら開始
		if (scene->GetSceneChangeAnimation()->IsFinished()) {
			scene->GetSceneChangeAnimation()->SetPhase(SceneChangeAnimation::Phase::Appearing);
			scene->SetIsRequestSceneChange(true);
		}
	}

	// シーンチェンジアニメーション進行完了後、実際の遷移
	if (scene->GetIsRequestSceneChange() && scene->GetSceneChangeAnimation()->IsFinished()) {
		if (StageScene::isDemoMode) {
			StageScene::isDemoMode = false;
			SceneManager::GetInstance()->ChangeScene(TITLE);
		} else {
			SceneManager::GetInstance()->ChangeScene(SCENE::CLEAR);
		}
		scene->SetIsRequestSceneChange(false);
	}

	// 挙動自体のスロー：このフレームでワールドを更新するか
	const bool doWorldUpdate = ShouldUpdateWorld();
	if (!doWorldUpdate) {
		return;
	}

	// 以降ワールド更新
	if (auto* follow = scene->GetFollowCamera()) {
		follow->Update();
	}

	// SkyDome
	if (auto* sky = scene->GetSkyDome().get()) {
		if (auto* follow = scene->GetFollowCamera()) {
			sky->SetCamera(follow->GetCamera());
		}
		sky->Update();
	}

	// ステージ（Tile/Block/Enemy）は StageManager 管理に統合
	if (auto* stageMgr = scene->GetStageManager()) {
		Player* player = scene->GetPlayer();
		FollowTopDownCamera* follow = scene->GetFollowCamera();
		stageMgr->Update(player, follow);
	}

	// プレイヤー
	if (auto* player = scene->GetPlayer()) {
		player->Update();
	}
}

void StageClearState::Exit(StageScene* scene) {
	(void)scene;
	// State を抜けたらスロー解除
	gSlowActive = false;
}

void StageClearState::Object3DDraw(StageScene* scene) {
	if (!scene) return;

	// 3Dオブジェクトの描画
	if (auto* stageMgr = scene->GetStageManager()) {
		stageMgr->Draw3D();
	}
	if (auto* sky = scene->GetSkyDome().get()) {
		sky->Draw();
	}
	if (auto* player = scene->GetPlayer()) {
		player->Draw();
	}
}

void StageClearState::SpriteDraw(StageScene* scene) { (void)scene; }

void StageClearState::ImGuiDraw(StageScene* scene) { (void)scene; }

void StageClearState::ParticleDraw(StageScene* scene) { (void)scene; }
