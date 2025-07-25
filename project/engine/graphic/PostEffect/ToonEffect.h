#pragma once
#include "Camera.h"
#include "DirectXCommon.h"
#include "Matrix4x4.h"
#include "PostEffectBase.h"
#include "ToonPSO.h"
#include "Vector3.h"

struct ToonParams {
	int stepCount = 3;
	float toonRate;
	Vector3 shadowColor;
	Vector3 highlightColor;
	float padding[2]; // HLSLと同じサイズになるように
};

class ToonEffect : public PostEffectBase {

public:
	ToonEffect();
	~ToonEffect();

	void Initialize() override;
	void Update() override;
	void DrawImGui() override;
	void Draw(ID3D12GraphicsCommandList* commandList) override;
	ToonParams* GetParams() { return toonParams_; }

public:
	void SetMainCamera(Camera* camera) override;

private:
	Camera* camera_ = nullptr; // メインカメラへのポインタ
	std::unique_ptr<ToonPSO> pso_;
	Microsoft::WRL::ComPtr<ID3D12Resource> toonCB_;
	ToonParams* toonParams_ = nullptr;
};