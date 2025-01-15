#pragma once

#include"IScene.h"
class StageScene :public IScene
{
public:
	void Initialize(Object3dCommon* object3dCommon, SpriteCommon* spriteCommon, WinApp* winApp, DirectXCommon* dxCommon)override;
	void Update()override;
	void Finalize()override;
	void Object3DDraw()override;
	void SpriteDraw()override;
	void ImGuiDraw()override;
};

