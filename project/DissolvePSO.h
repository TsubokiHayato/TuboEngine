#pragma once
#include "PostEffectPSOBase.h"

class DissolvePSO : public PostEffectPSOBase
{
public:
    // 初期化
    void Initialize(DirectXCommon* dxCommon) override;

    // グラフィックスパイプラインの作成
    void CreateGraphicPipeline();
    // 描画設定
    void CreateRootSignature() override;
};


