#include "HpUI.h"
#include "application/Character/Player/Player.h"

void HpUI::Initialize(const std::string& frameTexturePath, const std::string& fillTexturePath, int maxHp) {
    frameTex_ = frameTexturePath; fillTex_ = fillTexturePath;
    maxHp_ = maxHp;
    currentHp_ = maxHp_;
    frameSprites_.clear(); fillSprites_.clear();
    frameSprites_.reserve(maxHp_);
    fillSprites_.reserve(maxHp_);
    for (int i = 0; i < maxHp_; ++i) {
        auto fr = std::make_unique<Sprite>();
        fr->Initialize(frameTexturePath);
        fr->AdjustTextureSize();
        fr->SetAnchorPoint({0.0f, 0.0f});
        frameSprites_.push_back(std::move(fr));
        auto fi = std::make_unique<Sprite>();
        fi->Initialize(fillTexturePath);
        fi->AdjustTextureSize();
        fi->SetAnchorPoint({0.0f, 0.0f});
        fillSprites_.push_back(std::move(fi));
    }
}

void HpUI::Update(const Player* player) {
    if (!player) return;
    currentHp_ = player->GetHP();
    if (currentHp_ < 0) currentHp_ = 0;
    if (currentHp_ > maxHp_) currentHp_ = maxHp_;

    // Update positions and sizes for all sprites
    for (int i = 0; i < maxHp_; ++i) {
        Vector2 basePos = { position_.x + i * spacing_, position_.y };
        auto& fr = frameSprites_[i];
        fr->SetPosition(basePos);
        fr->SetSize({32.0f * scale_, 32.0f * scale_});
        fr->Update();
        auto& fi = fillSprites_[i];
        fi->SetPosition({ basePos.x + 2.0f * scale_, basePos.y + 2.0f * scale_ });
        fi->SetSize({ (32.0f - 4.0f) * scale_, (32.0f - 4.0f) * scale_ });
        fi->Update();
    }
}

void HpUI::Draw() {
    if (frameSprites_.empty()) return;
    for (int i = 0; i < maxHp_; ++i) {
        frameSprites_[i]->Draw();
        if (i < currentHp_) {
            fillSprites_[i]->Draw();
        }
    }
}
