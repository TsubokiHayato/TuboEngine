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
#include "Collider/CollisionManager.h"
#include "Collider/Collider.h"

#include "Camera/FollowTopDownCamera.h"

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
#include "application/UI/HpUI.h"
#include "application/UI/EnemyHpUI.h"

#include <string>
#include <vector>


class StageScene : public IScene {
public:
	/// <summary>
	/// 初期化
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
	Camera* GetMainCamera() const { return followCamera->GetCamera(); }

	void ChangeNextScene(int sceneNo) { SceneManager::GetInstance()->ChangeScene(sceneNo); }

	/// <summary>
	/// 全ての衝突をチェック
	/// </summary>
	void CheckAllCollisions();

public:
	///----------------------------------------------------------------------------------------
	///				引き渡し用変数
	///-----------------------------------------------------------------------------------------

	Player* GetPlayer() const { return player_.get(); }
	MapChipField* GetMapChipField() const { return mapChipField_.get(); }
	std::vector<std::unique_ptr<Block>>& GetBlocks() { return blocks_; }
	std::unique_ptr<Tile>& GetTile() { return tile_; }
	std::vector<std::unique_ptr<Enemy>>& GetEnemies() { return enemies; }

	FollowTopDownCamera* GetFollowCamera() const { return followCamera.get(); }
	std::string& GetMapChipCsvFilePath() { return mapChipCsvFilePath_; }
	std::unique_ptr<SkyDome>& GetSkyDome() { return skyDome_; }

	StageStateManager* GetStageStateManager() { return stateManager_.get(); }
	SceneChangeAnimation* GetSceneChangeAnimation() { return sceneChangeAnimation_.get(); }
	bool GetIsRequestSceneChange() const { return isRequestSceneChange; }
	void SetIsRequestSceneChange(bool request) { isRequestSceneChange = request; }

	// ------------------------
	// Multi-stage layout
	// ------------------------
	struct StageBounds {
		float left{};
		float right{};
		float bottom{};
		float top{};
	};

	struct StageInstance {
		std::string csvPath;
		Vector3 origin{0.0f, 0.0f, 0.0f};
		bool visible = true;

		std::unique_ptr<MapChipField> field;
		std::vector<std::unique_ptr<Block>> blocks;
		std::vector<std::unique_ptr<Enemy>> enemies;
		std::unique_ptr<Tile> tile;

		// Stage[0]用（Playerチップ探索結果）
		int playerMapX = -1;
		int playerMapY = -1;

		StageBounds boundsWorld{};
	};

	std::vector<StageInstance>& GetStageInstances() { return stageInstances_; }
	const std::vector<StageInstance>& GetStageInstances() const { return stageInstances_; }

	// デバッグ表示用：Stage[1..] のプレビューを描画するか
	bool GetDrawPreviewStages() const { return drawPreviewStages_; }
	void SetDrawPreviewStages(bool draw) { drawPreviewStages_ = draw; }

	// ...existing code...

private:
	///----------------------------------------------------------------------------------------
	///				メンバ変数
	///----------------------------------------------------------------------------------------

	std::unique_ptr<Audio> audio = nullptr;

	std::unique_ptr<CollisionManager> collisionManager_;

	std::unique_ptr<FollowTopDownCamera> followCamera;
	std::unique_ptr<Camera> camera = nullptr;
	Vector3 cameraPosition = {0.0f, 0.0f, -5.0f};
	Vector3 cameraRotation = {0.0f, 0.0f, 0.0f};
	Vector3 cameraScale = {1.0f, 1.0f, 1.0f};

	std::unique_ptr<Player> player_ = nullptr;
	std::unique_ptr<Enemy> enemy_ = nullptr;
	std::vector<std::unique_ptr<Enemy>> enemies;

	std::unique_ptr<SkyBox> skyBox_ = nullptr;

	std::unique_ptr<MapChipField> mapChipField_ = nullptr;
	std::string mapChipCsvFilePath_ = "Resources/MapChip.csv";

	std::vector<std::unique_ptr<Block>> blocks_;

	std::unique_ptr<Tile> tile_;

	std::unique_ptr<StageStateManager> stateManager_;

	std::unique_ptr<SkyDome> skyDome_ = nullptr;

	std::unique_ptr<SceneChangeAnimation> sceneChangeAnimation_ = nullptr;
	bool isRequestSceneChange = false;

	std::unique_ptr<HpUI> hpUI_;
	std::unique_ptr<EnemyHpUI> enemyHpUI_;

	// Multi-stage layout data (debug / editor)
	bool useMultiStageLayout_ = true;
	bool drawPreviewStages_ = true;
	std::vector<StageInstance> stageInstances_;

};