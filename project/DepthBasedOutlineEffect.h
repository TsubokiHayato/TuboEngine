#pragma once
#include "PostEffectBase.h"
#include "DepthBasedOutlinePSO.h"
#include"MT_Matrix.h"
#include "DirectXCommon.h"

// 定数バッファ構造体（projectionMatrix用）
struct DepthBasedOutlineMaterialCB {
    Matrix4x4 projectionMatrix;
};

class DepthBasedOutlineEffect : public PostEffectBase {
public:
    void Initialize() override;
    void Draw(ID3D12GraphicsCommandList* commandList) override;
    void Update() override;
    void DrawImGui() override;


private:
    std::unique_ptr<DepthBasedOutlinePSO> pso_;
    Microsoft::WRL::ComPtr<ID3D12Resource> materialCB_; // 定数バッファ
    DepthBasedOutlineMaterialCB* materialCBData_ = nullptr;
};
