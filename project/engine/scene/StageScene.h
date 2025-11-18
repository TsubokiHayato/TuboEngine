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

#include "MapChip/MapChipField.h"

#include "SkyBox.h"
#include "StageState/StageStateManager.h"
#include "Tile/Tile.h"
#include "SkyDome/SkyDome.h"

#include "Animation/SceneChangeAnimation.h"


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

	


	/// <summary>
	/// 
	/// </summary>
	/// <param name="sceneNo"></param>
	void ChangeNextScene(int sceneNo) { SceneManager::GetInstance()->ChangeScene(sceneNo); }

	void CheckAllCollisions();

public:
	///----------------------------------------------------------------------------------------
	///				引き渡し用変数
	///-----------------------------------------------------------------------------------------

	Player* GetPlayer() const { return player_.get(); }
	MapChipField* GetMapChipField() const { return mapChipField_.get(); }
	std::vector<std::unique_ptr<Block>>& GetBlocks() { return blocks_; }
	std::vector<std::unique_ptr<Tile>>& GetTiles() { return tiles_; }
	std::vector<std::unique_ptr<Enemy>>& GetEnemies() { return enemies; }

	FollowTopDownCamera* GetFollowCamera() const { return followCamera.get(); }
	std::string& GetMapChipCsvFilePath() { return mapChipCsvFilePath_; }
	std::unique_ptr<SkyDome>& GetSkyDome() { return skyDome_; }

	StageStateManager* GetStageStateManager() { return stateManager_.get(); }
	SceneChangeAnimation* GetSceneChangeAnimation() { return sceneChangeAnimation_.get(); }
	bool GetIsRequestSceneChange() const { return isRequestSceneChange; }
	void SetIsRequestSceneChange(bool request) { isRequestSceneChange = request; }

private:
	///----------------------------------------------------------------------------------------
	///				メンバ変数
	///----------------------------------------------------------------------------------------

	/// Audio///

	std::unique_ptr<Audio> audio = nullptr;

	/// Collider ///
	std::unique_ptr<CollisionManager> collisionManager_;

	/// Camera ///
	std::unique_ptr<FollowTopDownCamera> followCamera;
	std::unique_ptr<Camera> camera = nullptr;
	Vector3 cameraPosition = {0.0f, 0.0f, -5.0f};
	Vector3 cameraRotation = {0.0f, 0.0f, 0.0f};
	Vector3 cameraScale = {1.0f, 1.0f, 1.0f};

	/// Player ///
	std::unique_ptr<Player> player_ = nullptr;
	/// Enemy ///
	std::unique_ptr<Enemy> enemy_ = nullptr;
	std::vector<std::unique_ptr<Enemy>> enemies; // Enemyリスト

	/// SkyBox ///
	std::unique_ptr<SkyBox> skyBox_ = nullptr;

	/// MapChipField ///
	std::unique_ptr<MapChipField> mapChipField_ = nullptr;
	std::string mapChipCsvFilePath_ = "Resources/MapChip.csv"; // マップチップCSVファイルパス

	/// Block ///
	std::vector<std::unique_ptr<Block>> blocks_;

	/// Tile ///
	std::vector<std::unique_ptr<Tile>> tiles_;

	/// StageStateManager ///
	std::unique_ptr<StageStateManager> stateManager_;

	/// SkyDome///
	std::unique_ptr<SkyDome> skyDome_ = nullptr;

	/// SceneChangeAnimation ///

	std::unique_ptr<SceneChangeAnimation> sceneChangeAnimation_ = nullptr;
	bool isRequestSceneChange = false;

};