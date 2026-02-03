#pragma once
#include "Sprite.h"
#include <memory>
#include <vector>

class Player;

class HpUI {
public:
    ~HpUI() = default;
    void Initialize(const std::string& frameTexturePath, const std::string& fillTexturePath, int maxHp);
    void Update(const Player* player);
    void Draw();

    void SetPosition(const Vector2& pos) { position_ = pos; alignRight_ = false; }
    void SetSpacing(float spacing) { spacing_ = spacing; }
    void SetScale(float scale) { scale_ = scale; }
    void SetAlignRight(bool enable, float marginPx = 20.0f) { alignRight_ = enable; rightMargin_ = marginPx; }

private:
    // Per-HP sprites
    std::vector<std::unique_ptr<Sprite>> frameSprites_;
    std::vector<std::unique_ptr<Sprite>> fillSprites_;

    int maxHp_ = 0;
    int currentHp_ = 0;
    float animatedHp_ = 0.0f;   // animates from previous HP to current HP
    float shrinkSpeed_ = 4.0f;  // icons per second shrink rate
    Vector2 position_ { 20.0f, 20.0f };
    float spacing_ = 36.0f; // pixel spacing
    float scale_ = 1.0f;    // uniform scale

    bool alignRight_ = false;
    float rightMargin_ = 20.0f;

    // cached paths (optional)
    std::string frameTex_;
    std::string fillTex_;
};
