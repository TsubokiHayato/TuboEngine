#include "EnemyHpUI.h"
#include "application/Character/Enemy/Enemy.h"
#include "Camera.h"
#include "WinApp.h"
#include "Matrix.h"
#include <algorithm>
#include <cmath>

static Vector2 WorldToScreen(const Vector3& world, const Camera* cam) {
    if (!cam) return {0,0};
    const Matrix4x4& vp = cam->GetViewProjectionMatrix();
    Vector3 v = TransformCoord(world, vp);
    int sw = (int)WinApp::GetInstance()->GetClientWidth();
    int sh = (int)WinApp::GetInstance()->GetClientHeight();
    Vector2 screen;
    screen.x = (v.x * 0.5f + 0.5f) * sw;
    screen.y = (-v.y * 0.5f + 0.5f) * sh;
    return screen;
}

static Vector4 RatioColor(float ratio) {
    if (ratio < 0.0f) ratio = 0.0f; if (ratio > 1.0f) ratio = 1.0f;
    if (ratio < 0.5f) { float t = ratio / 0.5f; return {1.0f, t, 0.0f, 1.0f}; }
    else { float t = (ratio - 0.5f) / 0.5f; return {1.0f - t, 1.0f, 0.0f, 1.0f}; }
}

void EnemyHpUI::Initialize(const std::string& frameTexturePath, const std::string& fillTexturePath) {
    frameTex_ = frameTexturePath; fillTex_ = fillTexturePath;
}

void EnemyHpUI::Update(const std::vector<std::unique_ptr<Enemy>>& enemies, Camera* cam) {
    bars_.resize(enemies.size());
    for (size_t i = 0; i < enemies.size(); ++i) {
        auto* e = enemies[i].get();
        if (!e || !e->GetIsAllive()) { bars_[i].visible = false; continue; }
        int maxHp = e->GetMaxHP(); if (maxHp <= 0) { bars_[i].visible = false; continue; }
        int currentHp = std::clamp(e->GetHP(), 0, maxHp);
        EnemyBar& bar = bars_[i];
        bar.maxHp = maxHp; bar.currentHp = currentHp; bar.visible = true;
        if ((int)bar.frames.size() != maxHp) {
            bar.frames.clear(); bar.fills.clear();
            bar.frames.reserve(maxHp); bar.fills.reserve(maxHp);
            for (int h = 0; h < maxHp; ++h) {
                auto fr = std::make_unique<Sprite>(); fr->Initialize(frameTex_); fr->AdjustTextureSize(); fr->SetAnchorPoint({0.0f, 0.0f}); bar.frames.push_back(std::move(fr));
                auto fi = std::make_unique<Sprite>(); fi->Initialize(fillTex_);  fi->AdjustTextureSize(); fi->SetAnchorPoint({0.0f, 0.0f}); bar.fills.push_back(std::move(fi));
            }
            bar.animatedHp = (float)currentHp; // init animated to current
        }
        // animate towards current
        if (bar.animatedHp > (float)currentHp) {
            bar.animatedHp -= shrinkSpeed_ * (1.0f/60.0f);
            if (bar.animatedHp < (float)currentHp) bar.animatedHp = (float)currentHp;
        } else if (bar.animatedHp < (float)currentHp) {
            bar.animatedHp = (float)currentHp;
        }
        // position above enemy head
        Vector3 pos = e->GetPosition(); pos.y += 2.5f;
        Vector2 screen = WorldToScreen(pos, cam);
        screen.x = std::round(screen.x); screen.y = std::round(screen.y);
        float iconSize = 32.0f * scale_;
        float totalWidth = iconSize * maxHp + spacing_ * (maxHp - 1);
        Vector2 base = { screen.x - totalWidth / 2.0f, screen.y + yOffset_ };
        bar.basePos = base;
        float ratio = maxHp > 0 ? bar.animatedHp / (float)maxHp : 0.0f;
        Vector4 fillCol = RatioColor(ratio);
        int fullIcons = (int)std::floor(bar.animatedHp);
        float partial = bar.animatedHp - (float)fullIcons;
        float innerW = (32.0f - 4.0f) * scale_;
        float innerH = (32.0f - 4.0f) * scale_;
        for (int h = 0; h < maxHp; ++h) {
            Vector2 p = { base.x + h * (iconSize + spacing_), base.y };
            auto& fr = bar.frames[h]; fr->SetPosition(p); fr->SetSize({iconSize, iconSize}); fr->Update();
            auto& fi = bar.fills[h]; fi->SetPosition({ p.x + 2.0f * scale_, p.y + 2.0f * scale_ });
            if (h < fullIcons) {
                fi->SetSize({ innerW, innerH });
            } else if (h == fullIcons && partial > 0.0f) {
                fi->SetSize({ innerW * partial, innerH });
            } else {
                fi->SetSize({ 0.0f, innerH });
            }
            fi->SetColor(fillCol);
            fi->Update();
        }
    }
}

void EnemyHpUI::Draw() {
    for (auto& bar : bars_) {
        if (!bar.visible) continue;
        for (int h = 0; h < bar.maxHp; ++h) {
            bar.frames[h]->Draw();
            bar.fills[h]->Draw();
        }
    }
}
