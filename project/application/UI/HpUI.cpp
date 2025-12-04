#include "HpUI.h"
#include "application/Character/Player/Player.h"
#include "WinApp.h"
#include <algorithm>

static Vector4 HPColor(float ratio) {
    if (ratio < 0.0f) ratio = 0.0f; if (ratio > 1.0f) ratio = 1.0f;
    if (ratio < 0.5f) {
        float t = ratio / 0.5f; // 0..1
        return {1.0f, t, 0.0f, 1.0f}; // red->yellow
    } else {
        float t = (ratio - 0.5f) / 0.5f; // 0..1
        return {1.0f - t, 1.0f, 0.0f, 1.0f}; // yellow->green
    }
}

void HpUI::Initialize(const std::string& frameTexturePath, const std::string& fillTexturePath, int maxHp) {
    maxHp_ = maxHp;
    currentHp_ = maxHp_;
    animatedHp_ = (float)maxHp_;
    frameTex_ = frameTexturePath; fillTex_ = fillTexturePath;
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
    currentHp_ = std::clamp(currentHp_, 0, maxHp_);

    // Animate animatedHp_ downwards towards currentHp_
    if (animatedHp_ > (float)currentHp_) {
        animatedHp_ -= shrinkSpeed_ * (1.0f/60.0f);
        if (animatedHp_ < (float)currentHp_) animatedHp_ = (float)currentHp_;
    } else if (animatedHp_ < (float)currentHp_) {
        // snap up quickly on heal
        animatedHp_ = (float)currentHp_;
    }

    // compute base position
    float iconSize = 32.0f * scale_;
    float totalWidth = iconSize * maxHp_ + spacing_ * (maxHp_ - 1);
    Vector2 base = position_;
    if (alignRight_) {
        int sw = (int)WinApp::GetInstance()->GetClientWidth();
        base.x = (float)sw - rightMargin_ - totalWidth;
    }

    float ratio = maxHp_ > 0 ? animatedHp_ / (float)maxHp_ : 0.0f;
    Vector4 fillColor = HPColor(ratio);

    for (int i = 0; i < maxHp_; ++i) {
        Vector2 p = { base.x + i * (iconSize + spacing_), base.y };
        // frame
        auto& fr = frameSprites_[i];
        fr->SetPosition(p);
        fr->SetSize({iconSize, iconSize});
        fr->Update();
        // fill
        auto& fi = fillSprites_[i];
        fi->SetPosition({ p.x + 2.0f * scale_, p.y + 2.0f * scale_ });
        // For full icons left of the last partial, draw full; last icon draws partial width that shrinks right->left
        float innerW = (32.0f - 4.0f) * scale_;
        float innerH = (32.0f - 4.0f) * scale_;
        int fullIcons = (int)std::floor(animatedHp_);
        float partial = animatedHp_ - (float)fullIcons;
        if (i < fullIcons) {
            fi->SetSize({ innerW, innerH });
        } else if (i == fullIcons && partial > 0.0f) {
            // shrink from right to left: set width to innerW*partial
            fi->SetSize({ innerW * partial, innerH });
        } else {
            // empty
            fi->SetSize({ 0.0f, innerH });
        }
        fi->SetColor(fillColor);
        fi->Update();
    }
}

void HpUI::Draw() {
    if (frameSprites_.empty()) return;
    for (int i = 0; i < maxHp_; ++i) {
        frameSprites_[i]->Draw();
        // Draw only if width > 0
        // size is already set in Update for partial icon
        if (i < maxHp_) {
            fillSprites_[i]->Draw();
        }
    }
}
