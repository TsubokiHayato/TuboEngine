#pragma once
#include <memory>
#include <vector>
#include "Sprite.h"
#include "Vector3.h"
class Enemy; // ベースEnemyに変更
class Camera;

class EnemyHpUI {
public:
    void Initialize(const std::string& frameTexturePath, const std::string& fillTexturePath);
    void Update(const std::vector<std::unique_ptr<Enemy>>& enemies, Camera* cam);
    void Draw();

    void SetScale(float s) { scale_ = s; }
    void SetSpacing(float s) { spacing_ = s; }
    void SetYOffset(float y) { yOffset_ = y; }

private:
    struct EnemyBar {
        std::vector<std::unique_ptr<Sprite>> frames; // per-HP frame
        std::vector<std::unique_ptr<Sprite>> fills;  // per-HP fill
        bool visible = false;
        int maxHp = 0;
        int currentHp = 0;
        float animatedHp = 0.0f; // animated HP for smooth shrink
        Vector2 basePos{};
    };

    std::vector<EnemyBar> bars_;
    std::string frameTex_;
    std::string fillTex_;

    float scale_ = 1.0f;
    float spacing_ = 36.0f; // pixel spacing between icons
    float yOffset_ = -40.0f; // screen-space vertical offset above enemy head
    float shrinkSpeed_ = 4.0f; // icons per second
};
