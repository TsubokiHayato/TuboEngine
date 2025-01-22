#pragma once
#include"IScene.h"
#include"Particle.h"
#include"Camera.h"
#include"ParticleEmitter.h"

class TitleScene :public IScene
{
public:
	void Initialize(Object3dCommon* object3dCommon, SpriteCommon* spriteCommon, ParticleCommon* particleCommon, WinApp* winApp, DirectXCommon* dxCommon)override;
	void Update()override;
	void Finalize()override;
	void Object3DDraw()override;
	void SpriteDraw()override;
	void ImGuiDraw()override;
	void ParticleDraw()override;

private:
	Object3dCommon* object3dCommon;
	SpriteCommon* spriteCommon;
	ParticleCommon* particleCommon;
	WinApp* winApp;
	DirectXCommon* dxCommon;

	std::unique_ptr <Particle> particle;
	std::unique_ptr<ParticleEmitter> particleEmitter_;

	Camera* camera;
	Vector3 cameraPosition = { 0.0f,0.0f,-5.0f };
	Vector3 cameraRotation = { 0.0f,0.0f,0.0f };
	Vector3 cameraScale = { 1.0f,1.0f,1.0f };



};

