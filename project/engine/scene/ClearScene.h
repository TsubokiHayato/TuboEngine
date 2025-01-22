#pragma once
#include"IScene.h"
class ClearScene :public IScene
{
public:
	void Initialize(Object3dCommon* object3dCommon, SpriteCommon* spriteCommon, ParticleCommon* particleCommon, WinApp* winApp, DirectXCommon* dxCommon);
	void Update()override;
	void Finalize()override;
	void Object3DDraw()override;
	void SpriteDraw()override;
	void ImGuiDraw()override;
	void ParticleDraw()override;
};

