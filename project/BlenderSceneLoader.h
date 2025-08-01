#pragma once
#include "Object3d.h"
#include "Transform.h"
#include "Vector3.h"
#include <memory>
#include <string>
#include <vector>
// レベルデータ構造体
struct LevelData {
	struct ObjectData {
		std::string fileName;
		Transform transform;
	};
	struct PlayerSpawnData {
		std::string fileName;
		Transform transform;
	};
	std::vector<ObjectData> objects;
	std::vector<PlayerSpawnData> playerSpawn;
	Transform cameraTransform;
};

class BlenderSceneLoader {
public:
	// シーンデータのロード
	void Load(const std::string& fileName);
	// Object3d生成
	void CreateObject();
	// 毎フレーム更新
	void Update();
	// 描画
	void Draw();

	void DrawImgui();

	void SetCamera(Camera* camera);

	// プレイヤースポーン情報取得
	const std::vector<LevelData::PlayerSpawnData>& getPlayerSpawns() const;
	bool HasPlayerSpawn() const;
	const Transform& GetCameraTransform() const { return levelData->cameraTransform; }

private:
	std::unique_ptr<LevelData> levelData;
	std::vector<std::unique_ptr<Object3d>> objects;
	Camera* camera = nullptr;
};
