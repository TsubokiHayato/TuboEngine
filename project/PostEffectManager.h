#include <vector>
#include <memory>
#include"PostEffectBase.h"
#include"OutlineEffect.h"

class PostEffectManager
{
public:
    void AddEffect(std::unique_ptr<PostEffectBase> effect);

    void InitializeAll(DirectXCommon* dxCommon);

    void UpdateAll();

    // エフェクト切り替え
    void SetCurrentEffect(size_t index);

    void DrawCurrent(ID3D12GraphicsCommandList* commandList);

    void DrawImGui();
    size_t GetEffectCount() const { return effects_.size(); }
    size_t GetCurrentIndex() const { return currentIndex_; }

public:

    void SetOutlineProjection(const Matrix4x4& projection) {
        if (outlineEffect_) {
            outlineEffect_->SetProjection(projection);
        }
    }

    // OutlineEffect の初期化や取得用メソッドも必要に応じて追加
    void SetOutlineEffect(OutlineEffect* effect) {
        outlineEffect_ = effect;
    }

private:
    OutlineEffect* outlineEffect_ = nullptr;
    std::vector<std::unique_ptr<PostEffectBase>> effects_;
    size_t currentIndex_ = 0;
};

