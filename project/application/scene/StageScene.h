#pragma once
#include "Audio.h"
#include "Camera.h"
#include "IScene.h"
#include "Input.h"
#include "Object3dCommon.h"
#include "SpriteCommon.h"
#include "AudioCommon.h"
#include "ImGuiManager.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "SceneManager.h"
#include <memory>
#include "engine/Collider/CollisionManager.h"
#include "engine/Collider/Collider.h"

#include "Camera/FollowTopDownCamera.h"
#include "DebugCamera.h"

#include "Block/Block.h"

#include "Bullet/Player/PlayerBullet.h"
#include "Character/Player/Player.h"

#include "Character/Enemy/Enemy.h"
#include "Character/Enemy/RushEnemy.h"

#include "MapChip/MapChipField.h"

#include "SkyBox.h"
#include "StageState/StageStateManager.h"
#include "Tile/Tile.h"
#include "SkyDome/SkyDome.h"

#include "Animation/SceneChangeAnimation.h"
#include "application/UI/HP/Player/HpUI.h"
#include "application/UI/HP/Enemy/EnemyHpUI.h"
#include "application/UI/Guide/GuideUI.h"

#include "application/Stage/StageManager.h" // 追加: StageManager の完全型定義

#include <string>
#include <vector>

/// <summary>
/// ステージプレイシーン。プレイヤー・敵・マップチップ・カメラ・UIなど
/// ステージ内のオブジェクトを統括し、更新・描画・当たり判定・シーン遷移を行う。
/// </summary>
class StageScene : public IScene {
public:
	// デモモードフラグ（タイトル画面で放置された場合にtrueになる）
	static bool isDemoMode;

	/// <summary>
	/// 初期化処理。
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize() override;

	/// <summary>
	/// 3Dオブジェクト描画
	/// </summary>
	void Object3DDraw() override;

	/// <summary>
	/// スプライト描画
	/// </summary>
	void SpriteDraw() override;

	/// <summary>
	/// ImGui描画
	/// </summary>
	void ImGuiDraw() override;

	/// <summary>
	/// パーティクル描画
	/// </summary>
	void ParticleDraw() override;

	/// <summary>
	/// メインカメラ取得
	/// </summary>
	TuboEngine::Camera* GetMainCamera() const { return followCamera->GetCamera(); }

	/// <summary>
	/// 指定シーン番号へ遷移する。
	/// </summary>
	void ChangeNextScene(int sceneNo) { SceneManager::GetInstance()->ChangeScene(sceneNo); }

	/// <summary>
	/// 全ての衝突をチェック
	/// </summary>
	void CheckAllCollisions();

public:
	///----------------------------------------------------------------------------------------
	///			引き渡し用変数
	///-----------------------------------------------------------------------------------------

	/// <summary>
	/// プレイヤーの参照を取得する。
	/// </summary>
	Player* GetPlayer() const { return player_.get(); }
	/// <summary>
	/// マップチップフィールドの参照を取得する。
	/// </summary>
	MapChipField* GetMapChipField() const { return mapChipField_.get(); }
	// Stage オブジェクトはすべて StageManager 管理に移行したため、
	// 旧 blocks_/tile_/enemies の Getter は削除済み。

	/// <summary>
	/// FollowCamera を取得する。
	/// </summary>
	FollowTopDownCamera* GetFollowCamera() const { return followCamera.get(); }
	/// <summary>
	/// MapChipCsvFilePath を取得する。
	/// </summary>
	std::string& GetMapChipCsvFilePath() { return mapChipCsvFilePath_; }
	/// <summary>
	/// SkyDome を取得する。
	/// </summary>
	std::unique_ptr<SkyDome>& GetSkyDome() { return skyDome_; }

	/// <summary>
	/// StageStateManager を取得する。
	/// </summary>
	StageStateManager* GetStageStateManager() { return stateManager_.get(); }
	/// <summary>
	/// SceneChangeAnimation を取得する。
	/// </summary>
	SceneChangeAnimation* GetSceneChangeAnimation() { return sceneChangeAnimation_.get(); }
	/// <summary>
	/// IsRequestSceneChange の取得・設定。
	/// </summary>
	bool GetIsRequestSceneChange() const { return isRequestSceneChange; }
	void SetIsRequestSceneChange(bool request) { isRequestSceneChange = request; }

	// 追加: 同一StageScene内のステート切替を、SceneChangeAnimationで
	// 『覆い→ステート切替→開場』として扱うためのリクエストAPI
	void RequestStateChangeWithTransition(StageType nextState);
	bool IsStateChangeTransitionActive() const { return isStateChangeTransitionActive_; }

	// ------------------------
	// Multi-stage layout
	// ------------------------
	/// <summary>
	/// マルチステージレイアウトにおける1ステージの矩形範囲（ワールド座標の左右下上）を表す。
	/// </summary>
	struct StageBounds {
		float left{};
		float right{};
		float bottom{};
		float top{};
	};

	/// <summary>
	/// StageManager を取得する。
	/// </summary>
	StageManager* GetStageManager() const { return stageManager_.get(); }

	/// <summary>
	/// DrawPreviewStages の取得・設定。
	/// </summary>
	bool GetDrawPreviewStages() const { return drawPreviewStages_; }
	void SetDrawPreviewStages(bool draw) { drawPreviewStages_ = draw; }

private:
	///----------------------------------------------------------------------------------------
	///			メンバ変数
	///----------------------------------------------------------------------------------------

	std::unique_ptr<TuboEngine::Audio> audio = nullptr;

	std::unique_ptr<CollisionManager> collisionManager_;

	std::unique_ptr<FollowTopDownCamera> followCamera;
	TuboEngine::DebugCamera debugCamera_; // デバッグ用フリーカメラ（F2でON/OFF）
	std::unique_ptr<TuboEngine::Camera> camera = nullptr;
	TuboEngine::Math::Vector3 cameraPosition = {0.0f, 0.0f, -5.0f};
	TuboEngine::Math::Vector3 cameraRotation = {0.0f, 0.0f, 0.0f};
	TuboEngine::Math::Vector3 cameraScale = {1.0f, 1.0f, 1.0f};

	std::unique_ptr<Player> player_ = nullptr;
	std::unique_ptr<Enemy> enemy_ = nullptr; 

	std::unique_ptr<TuboEngine::SkyBox> skyBox_ = nullptr;

	std::unique_ptr<MapChipField> mapChipField_ = nullptr;
	std::string mapChipCsvFilePath_ = "Resources/Stage/MapChip.csv";
	// Demo専用CSVパス（デモプレイ時にこちらを読み込む）
	std::string demoMapChipCsvFilePath_ = "Resources/Stage/Demo.csv";

public:
	// Demo用CSVパス取得
	const std::string& GetDemoMapChipCsvFilePath() const { return demoMapChipCsvFilePath_; }

	// 旧: blocks_/tile_/enemies は StageManager 統合により削除

	std::unique_ptr<StageStateManager> stateManager_;

	std::unique_ptr<SkyDome> skyDome_ = nullptr;

	std::unique_ptr<SceneChangeAnimation> sceneChangeAnimation_ = nullptr;
	bool isRequestSceneChange = false;
	// シーンチェンジで遷移先を一時保持する（SceneChangeAnimation 完了時に遷移する）
	int pendingNextSceneNo_ = -1;

public:
	// シーンチェンジ遷移先設定 (SceneChangeAnimation を使う場合)
	void SetPendingNextScene(int sceneNo) { pendingNextSceneNo_ = sceneNo; }
	int GetPendingNextScene() const { return pendingNextSceneNo_; }

	// 追加: ステート切替(リスタート等)用のトランジション状態
	bool isStateChangeTransitionActive_ = false;
	StageType pendingNextState_ = StageType::Ready;
	bool startDisappearingAfterStateChange_ = false;

	std::unique_ptr<HpUI> hpUI_;
	std::unique_ptr<EnemyHpUI> enemyHpUI_;
	std::unique_ptr<GuideUI> guideUI_;

	// Demo overlay sprites
	std::unique_ptr<TuboEngine::Sprite> demoLabelSprite_ = nullptr;     // "ーDEMOー"
	std::unique_ptr<TuboEngine::Sprite> demoPressAnySprite_ = nullptr;  // "PRESS ANY BUTTON"

	// Multi-stage layout data (debug / editor)
	bool useMultiStageLayout_ = true;
	bool drawPreviewStages_ = true;

	std::unique_ptr<StageManager> stageManager_;
};