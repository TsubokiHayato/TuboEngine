#pragma once
#include<memory>
#include"IScene.h"
class SceneManager
{
public:
	/// <summary>
	/// シングルトンインスタンス取得
	/// </summary>
	static SceneManager* GetInstance() {
		
		if (!instance) {
			instance = new SceneManager();
		}
		return instance;
	}

private:
	// コンストラクタ・デストラクタ・コピー禁止
	static SceneManager* instance ;
	SceneManager() = default;
	~SceneManager() = default;
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;


public:
	//初期化
	void Initialize();
	//更新
	void Update();
	//終了処理
	void Finalize();
	//オブジェクト3D描画
	void Object3DDraw();
	//スプライト描画
	void SpriteDraw();
	//ImGui描画
	void ImGuiDraw();
	// パーティクル描画
	void ParticleDraw();

	// シーン切り替え
	void ChangeScene(int sceneNo);

	// MainCamera取得
	Camera* GetMainCamera() const {
		if (currentScene) {
			return currentScene->GetMainCamera();
		}
		return nullptr;
	}

private:

	//現在のシーン
	std::unique_ptr<IScene> currentScene = nullptr;
	//前のシーン
	std::unique_ptr<IScene> prevScene = nullptr;
	//現在のシーン番号
	int currentSceneNo = 0;
	//前のシーン番号
	int prevSceneNo = 0;
};

