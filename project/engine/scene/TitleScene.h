#pragma once
#include "Camera.h"
#include "IScene.h"
#include "Particle.h"
#include "ParticleEmitter.h"
#include <random>
#include "BlenderSceneLoader.h" 

class TitleScene : public IScene {
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
	/// カメラの取得
	/// </summary>
	Camera* GetMainCamera() const { return camera.get(); }

private:
	Object3dCommon* object3dCommon;
	SpriteCommon* spriteCommon;
	ParticleCommon* particleCommon;
	WinApp* winApp;
	DirectXCommon* dxCommon;

private:
	std::unique_ptr<Camera> camera;
	Vector3 cameraPosition = {0.0f, 0.0f, -5.0f};
	Vector3 cameraRotation = {0.0f, 0.0f, 0.0f};
	Vector3 cameraScale = {1.0f, 1.0f, 1.0f};

	std::unique_ptr<BlenderSceneLoader> blenderSceneLoader_; // ←追加
};
