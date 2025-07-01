#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <cassert>
#include "DirectXCommon.h"

class PostEffectBase
{
public:
    virtual ~PostEffectBase() = default;

    // 初期化
    virtual void Initialize(DirectXCommon* dxCommon) = 0;

    // パラメータ更新（ImGuiやアニメーション用）
    virtual void Update() {}

	// ImGui描画
	virtual void DrawImGui() {}

    // 描画
    virtual void Draw(ID3D12GraphicsCommandList* commandList) = 0;
};
