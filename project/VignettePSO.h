#pragma once
#include "PostEffectPSOBase.h"

class VignettePSO : public PostEffectPSOBase
{
public:
    // 初期化
    void Initialize(DirectXCommon* dxCommon) override;

    // グラフィックスパイプラインの作成
    void CreateGraphicPipeline();

    void CreateRootSignature() override;
};
