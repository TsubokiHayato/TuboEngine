#include "PostEffectBase.h"
#include "NonePSO.h"


class NoneEffect : public PostEffectBase
{
public:
    void Initialize(DirectXCommon* dxCommon) override;
    void Draw(ID3D12GraphicsCommandList* commandList) override;
private:
    std::unique_ptr<NonePSO> pso_;
};
