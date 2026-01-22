#pragma once
#include"WinApp.h"
#include"DirectXCommon.h"
#include"IScene.h"

#include"Matrix.h"
#include "Input.h"
#include"Audio.h"
#include"Camera.h"
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

// パーティクル
#include "engine/graphic/Particle/ParticleManager.h"
#include "engine/graphic/Particle/PrimitiveEmitter.h"
#include "engine/graphic/Particle/RingEmitter.h"
#include "engine/graphic/Particle/CylinderEmitter.h"
#include "engine/graphic/Particle/OriginalEmitter.h"

#undef min
#undef max

# define PI 3.14159265359f

class DebugScene :public IScene {

public:
	void Initialize() override;
	void Update() override;
	void Finalize() override;
	void Object3DDraw() override;
	void SpriteDraw() override;
	void ImGuiDraw() override;
	void ParticleDraw() override;

	Camera* GetMainCamera() const { return camera.get(); }

private:
	std::unique_ptr<Audio> audio = nullptr;

	std::unique_ptr<Camera> camera = nullptr;
	Vector3 cameraPosition = { 0.0f,1.0f,-15.0f };
	Vector3 cameraRotation = { 0.0f,0.0f,0.0f };
	Vector3 cameraScale = { 1.0f,1.0f,1.0f };

	int lightType = 0;
	Vector3 lightDirection = { 0.0f,-1.0f,0.0f };
	Vector4 lightColor = { 1.0f,1.0f,1.0f,1.0f };
	float intensity = 1.0f;
	float shininess = 1.0f;

	Vector3 pointLightPosition = { 0.0f,0.0f,0.0f };
	Vector4 pointLightColor = { 1.0f,1.0f,1.0f,1.0f };
	float pointLightIntensity = 1.0f;

	std::unique_ptr<SkyBox> skyBox = nullptr;

	std::unique_ptr<SceneChangeAnimation> sceneChangeAnimation = nullptr;
	bool isRequestSceneChange = false;

	// 変更: 生ポインタ配列 → 名前配列
	std::vector<std::string> emitterNames_;
	bool particleInitialized_ = false;
};

