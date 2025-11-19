#pragma once
#include <vector>
#include <memory>
#include <string>
#include "IParticleEmitter.h"

class ParticleManager {
public:
    static ParticleManager* GetInstance() {
        static ParticleManager inst;
        return &inst; 
    }

    void Update(float dt, Camera* defaultCam);
    void Draw();
    void DrawImGui();

    template<typename EmitterT>
    EmitterT* CreateEmitter(const ParticlePreset& preset) {
        auto ptr = std::make_unique<EmitterT>();
        ptr->Initialize(preset);
        EmitterT* raw = ptr.get();
        emitters_.push_back(std::move(ptr));
        return raw;
    }

    IParticleEmitter* Find(const std::string& name);
    void Remove(const std::string& name);

    // 保存/読込（簡易テキスト形式）
    void SaveAll(const std::string& filePath);
    void LoadAll(const std::string& filePath);

private:
    std::vector<std::unique_ptr<IParticleEmitter>> emitters_;
};