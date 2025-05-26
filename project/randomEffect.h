#pragma once
#include "PostEffectBase.h"
#include"randomPSO.h"

// randomEffect.h など
struct RandomParams
{
	float time;
	float randomValue; // 追加
	float padding[2];  // 16バイトアライメント
};

class randomEffect : public PostEffectBase
{

public:

	randomEffect();
	~randomEffect();
	void Initialize(DirectXCommon* dxCommon) override;
	void Update() override;
	void DrawImGui() override;
	void Draw(ID3D12GraphicsCommandList* commandList) override;
	// ImGui等でパラメータを外部から変更したい場合
	RandomParams* GetParams() { return params_; }

private:
	std::unique_ptr<randomPSO> pso_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cbResource_;
	RandomParams* params_ = nullptr;

};

