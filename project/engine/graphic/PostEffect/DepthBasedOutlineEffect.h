#pragma once
#include "Camera.h"
#include "DepthBasedOutlinePSO.h"
#include "DirectXCommon.h"
#include "MT_Matrix.h"
#include "PostEffectBase.h"

// 定数バッファ構造体（projectionMatrix用）
struct ToonDepthOutlineParams {
	Matrix4x4 projectionInverse;
	Vector4 outlineColor; // アウトラインの色
	float outlineThickness; // アウトラインの太さ
	float outlineDepthThreshold; // アウトラインの深度しきい値
};

class DepthBasedOutlineEffect : public PostEffectBase {
public:
	void Initialize() override;
	void Draw(ID3D12GraphicsCommandList* commandList) override;
	void Update() override;
	void DrawImGui() override;

public:
	void SetMainCamera(Camera* camera) override;
	// 定数バッファの取得
	ID3D12Resource* GetMaterialCB() const { return materialCB_.Get(); }

private:
	Camera* camera_ = nullptr; // メインカメラへのポインタ
	std::unique_ptr<DepthBasedOutlinePSO> pso_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialCB_; // 定数バッファ
	ToonDepthOutlineParams* materialCBData_ = nullptr;
};
