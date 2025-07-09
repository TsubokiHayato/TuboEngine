#include <vector>
#include <memory>
#include"PostEffectBase.h"

class PostEffectManager
{
public:
    void AddEffect(std::unique_ptr<PostEffectBase> effect);

    void InitializeAll();

    void UpdateAll();

    // エフェクト切り替え
    void SetCurrentEffect(size_t index);

    void DrawCurrent(ID3D12GraphicsCommandList* commandList);

    void DrawImGui();
    size_t GetEffectCount() const { return effects_.size(); }
    size_t GetCurrentIndex() const { return currentIndex_; }

private:
    std::vector<std::unique_ptr<PostEffectBase>> effects_;
    size_t currentIndex_ = 0;
};

