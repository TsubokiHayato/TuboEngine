#pragma once
#include "PostEffectBase.h"
#include "RadialBlurPSO.h"
#include"Vector2.h"

struct RadialBlurParams
{
	Vector2 radialBlurCenter; // 中心座標
	float radialBlurPower;  // 効果の強さ
	float pad[2]; // 16バイトアライメント

};

class RadialBlurEffect : public PostEffectBase
{
public:
	RadialBlurEffect();
	~RadialBlurEffect();
	void Initialize(DirectXCommon* dxCommon) override;
	void Update() override;
	void DrawImGui() override;
	void Draw(ID3D12GraphicsCommandList* commandList) override;

private:
	std::unique_ptr<RadialBlurPSO> pso_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cbResource_;
	RadialBlurParams* params_ = nullptr;

};

