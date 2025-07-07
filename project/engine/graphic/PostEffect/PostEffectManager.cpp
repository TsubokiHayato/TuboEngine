#include "PostEffectManager.h"

void PostEffectManager::AddEffect(std::unique_ptr<PostEffectBase> effect) {
    effects_.emplace_back(std::move(effect));
}

void PostEffectManager::InitializeAll() {
    for (auto& e : effects_) e->Initialize();
}

void PostEffectManager::UpdateAll() {
    for (auto& e : effects_) e->Update();
}

void PostEffectManager::SetCurrentEffect(size_t index) {
    if (index < effects_.size()) {
        currentIndex_ = index;
    }
}

void PostEffectManager::DrawCurrent(ID3D12GraphicsCommandList* commandList) {
    if (currentIndex_ < effects_.size()) {
        effects_[currentIndex_]->Draw(commandList);
    }
}

void PostEffectManager::DrawImGui() {
    if (currentIndex_ < effects_.size()) {
        effects_[currentIndex_]->DrawImGui();
    }
}

