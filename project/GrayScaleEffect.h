#include "PostEffectBase.h"
#include "GrayScalePSO.h"


class GrayScaleEffect : public PostEffectBase
{
public:
    void Initialize(DirectXCommon* dxCommon) override;
    void Draw(ID3D12GraphicsCommandList* commandList) override;
private:
    std::unique_ptr<GrayScalePSO> pso_;
};
