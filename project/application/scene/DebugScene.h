#pragma once
#include"WinApp.h"
#include"DirectXCommon.h"
#include"IScene.h"

#include"Matrix.h"
#include "Input.h"
#include"Audio.h"
#include"Camera.h"
#include"DebugCamera.h"
#include"Sprite.h"
#include"Object3d.h"
#include"Model.h"
#include <iostream>
#include <algorithm>

#include"SpriteCommon.h"
#include"Object3dCommon.h"
#include"TextureManager.h"
#include"ModelManager.h"
#include"AudioCommon.h"
#include"ImGuiManager.h"

#include"SkyBox.h"
#include"Animation/SceneChangeAnimation.h"

#include "TextManager.h"

// パーティクル
#include "engine/graphic/Particle/ParticleManager.h"
#include "engine/graphic/Particle/Effects/Primitive/PrimitiveEmitter.h"
#include "Effects/Ring/RingEmitter.h"
#include "engine/graphic/Particle/Effects/Original/OriginalEmitter.h"
#include "Effects/Cylinder/CylinderEmitter.h"

// SPH 流体シミュレーション
#include "engine/graphic/SPH/SphSimulator.h"
#undef min
#undef max

# define PI 3.14159265359f

/// <summary>
/// エンジン機能の動作確認を行うデバッグ用シーン。
/// </summary>
class DebugScene :public IScene {

public:
	/// <summary>
	/// 初期化処理。
	/// </summary>
	void Initialize() override;
	/// <summary>
	/// 更新処理。
	/// </summary>
	void Update() override;
	/// <summary>
	/// 終了処理。
	/// </summary>
	void Finalize() override;
	/// <summary>
	/// 3Dオブジェクト描画。
	/// </summary>
	void Object3DDraw() override;
	/// <summary>
	/// スプライト描画。
	/// </summary>
	void SpriteDraw() override;
	/// <summary>
	/// ImGui描画。
	/// </summary>
	void ImGuiDraw() override;
	/// <summary>
	/// パーティクル描画。
	/// </summary>
	void ParticleDraw() override;

	/// <summary>
	/// メインカメラを取得する。
	/// </summary>
	TuboEngine::Camera* GetMainCamera() const { return camera.get(); }

private:

	std::unique_ptr<TuboEngine::Camera> camera = nullptr;
	TuboEngine::DebugCamera debugCamera_; // デバッグ用フリーカメラ（F2でON/OFF）
	TuboEngine::Math::Vector3 cameraPosition = {0.0f, 1.0f, -15.0f};
	TuboEngine::Math::Vector3 cameraRotation = {0.0f, 0.0f, 0.0f};
	TuboEngine::Math::Vector3 cameraScale = {1.0f, 1.0f, 1.0f};

	int lightType = 0;
	TuboEngine::Math::Vector3 lightDirection = {0.0f, -1.0f, 0.0f};
	TuboEngine::Math::Vector4 lightColor = {1.0f, 1.0f, 1.0f, 1.0f};
	float intensity = 1.0f;
	float shininess = 1.0f;

	TuboEngine::Math::Vector3 pointLightPosition = {0.0f, 0.0f, 0.0f};
	TuboEngine::Math::Vector4 pointLightColor = {1.0f, 1.0f, 1.0f, 1.0f};
	float pointLightIntensity = 1.0f;

	// テキストテスト用
	TuboEngine::TextObject* testText_ = nullptr;

	// 変更: 生ポインタ配列 → 名前配列
	std::vector<std::string> emitterNames_;
	bool particleInitialized_ = false;

	// SPH 流体シミュレーション
	std::unique_ptr<SphSimulator> sphSimulator_;
};

