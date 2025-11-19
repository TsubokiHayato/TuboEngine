#include "ParticleManager.h"
#include "ImGuiManager.h"
#include "Camera.h"
#include <externals/nlohmann/json.hpp>
#include <fstream>
#include <algorithm>
#include "PrimitiveEmitter.h"
#include "RingEmitter.h"
#include "CylinderEmitter.h"
#include "OriginalEmitter.h"

void ParticleManager::Update(float dt, Camera* cam) {
    for (auto& e : emitters_) {
        auto& preset = e->GetPreset();
        if (preset.autoEmit && preset.emitRate > 0.0f) {
            // 個別蓄積（衝突回避のため static を廃止）
            float interval = 1.0f / preset.emitRate;
            preset._emitAccum += dt;
            while (preset._emitAccum >= interval) {
                e->Emit(1);
                preset._emitAccum -= interval;
            }
        }
        e->Update(dt, cam);
    }
}

void ParticleManager::Draw() {
    auto* cmd = DirectXCommon::GetInstance()->GetCommandList().Get();
    for (auto& e : emitters_) {
        e->Draw(cmd);
    }
}

void ParticleManager::DrawImGui() {
#ifdef USE_IMGUI
    if (ImGui::Begin("Particle Manager")) {
        if (ImGui::Button("Save All")) { SaveAll("Resources/Particles/all.json"); }
        ImGui::SameLine();
        if (ImGui::Button("Load All")) { LoadAll("Resources/Particles/all.json"); }

        static int newType = 0;
        ImGui::Combo("New Type", &newType, "Primitive\0Ring\0Cylinder\0Original\0");

        // 安全な入力バッファ
        static char nameBuf[64] = "Emitter";
        static char texBuf[128] = "particle.png";
        static ParticlePreset tmpPreset{};
        ImGui::InputText("Name", nameBuf, sizeof(nameBuf));
        ImGui::InputText("Texture", texBuf, sizeof(texBuf));
        tmpPreset.name = nameBuf;
        tmpPreset.texture = texBuf;
        ImGui::Checkbox("AutoEmit", &tmpPreset.autoEmit);
        ImGui::DragFloat("EmitRate", &tmpPreset.emitRate, 0.2f, 0.0f, 300.0f);
        ImGui::DragInt("BurstCount", reinterpret_cast<int*>(&tmpPreset.burstCount), 1, 1, 1000);
        ImGui::DragFloat2("LifeRange", &tmpPreset.lifeMin, 0.01f, 0.01f, 10.0f);
        ImGui::DragFloat3("PosMin", &tmpPreset.posMin.x, 0.01f);
        ImGui::DragFloat3("PosMax", &tmpPreset.posMax.x, 0.01f);
        ImGui::DragFloat3("VelMin", &tmpPreset.velMin.x, 0.01f);
        ImGui::DragFloat3("VelMax", &tmpPreset.velMax.x, 0.01f);
        ImGui::DragFloat3("ScaleMin", &tmpPreset.scaleMin.x, 0.01f);
        ImGui::DragFloat3("ScaleMax", &tmpPreset.scaleMax.x, 0.01f);
        ImGui::ColorEdit4("ColMin", &tmpPreset.colMin.x);
        ImGui::ColorEdit4("ColMax", &tmpPreset.colMax.x);

        if (ImGui::Button("Create")) {
            switch (newType) {
            case 0: CreateEmitter<PrimitiveEmitter>(tmpPreset); break;
            case 1: CreateEmitter<RingEmitter>(tmpPreset); break;
            case 2: CreateEmitter<CylinderEmitter>(tmpPreset); break;
            case 3: CreateEmitter<OriginalEmitter>(tmpPreset); break;
            }
        }

        ImGui::Separator();
        for (auto& e : emitters_) {
            if (ImGui::TreeNode(e->GetName().c_str())) {
                auto& p = e->GetPreset();
                ImGui::Checkbox(("AutoEmit##" + p.name).c_str(), &p.autoEmit);
                ImGui::DragFloat(("EmitRate##" + p.name).c_str(), &p.emitRate, 0.1f, 0.0f, 400.0f);
                ImGui::DragInt(("Burst##" + p.name).c_str(), reinterpret_cast<int*>(&p.burstCount), 1, 1, 2000);
                ImGui::DragFloat2(("LifeRange##" + p.name).c_str(), &p.lifeMin, 0.01f, 0.01f, 20.0f);
                ImGui::ColorEdit4(("ColMin##" + p.name).c_str(), &p.colMin.x);
                ImGui::ColorEdit4(("ColMax##" + p.name).c_str(), &p.colMax.x);
                if (ImGui::Button(("Emit Burst##" + p.name).c_str())) {
                    e->Emit(p.burstCount);
                }
                ImGui::SameLine();
                if (ImGui::Button(("Delete##" + p.name).c_str())) {
                    Remove(p.name);
                    ImGui::TreePop();
                    break;
                }
                ImGui::TreePop();
            }
        }
    }
    ImGui::End();
#endif
}

IParticleEmitter* ParticleManager::Find(const std::string& name) {
    for (auto& e : emitters_) if (e->GetName() == name) return e.get();
    return nullptr;
}

void ParticleManager::Remove(const std::string& name) {
    emitters_.erase(std::remove_if(emitters_.begin(), emitters_.end(),
        [&](auto& u) { return u->GetName() == name; }), emitters_.end());
}

void ParticleManager::SaveAll(const std::string& path) {
    nlohmann::json root;
    root["version"] = 1;
    for (auto& e : emitters_) {
        const auto& p = e->GetPreset();
        std::string type = "Primitive";
        if (dynamic_cast<RingEmitter*>(e.get())) type = "Ring";
        else if (dynamic_cast<CylinderEmitter*>(e.get())) type = "Cylinder";
        else if (dynamic_cast<OriginalEmitter*>(e.get())) type = "Original";

        nlohmann::json j{
            {"type", type},
            {"name", p.name},
            {"texture", p.texture},
            {"maxInstances", p.maxInstances},
            {"billboard", p.billboard},
            {"emitRate", p.emitRate},
            {"autoEmit", p.autoEmit},
            {"burstCount", p.burstCount},
            {"lifeMin", p.lifeMin},
            {"lifeMax", p.lifeMax},
            {"posMin", {p.posMin.x,p.posMin.y,p.posMin.z}},
            {"posMax", {p.posMax.x,p.posMax.y,p.posMax.z}},
            {"velMin", {p.velMin.x,p.velMin.y,p.velMin.z}},
            {"velMax", {p.velMax.x,p.velMax.y,p.velMax.z}},
            {"scaleMin",{p.scaleMin.x,p.scaleMin.y,p.scaleMin.z}},
            {"scaleMax",{p.scaleMax.x,p.scaleMax.y,p.scaleMax.z}},
            {"colMin",{p.colMin.x,p.colMin.y,p.colMin.z,p.colMin.w}},
            {"colMax",{p.colMax.x,p.colMax.y,p.colMax.z,p.colMax.w}}
        };
        root["emitters"].push_back(j);
    }
    std::ofstream ofs(path);
    ofs << root.dump(2);
}

void ParticleManager::LoadAll(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs) return;
    nlohmann::json root;
    try { root = nlohmann::json::parse(ifs); }
    catch (...) { return; }
    emitters_.clear();
    if (!root.contains("emitters")) return;
    for (auto& j : root["emitters"]) {
        ParticlePreset p;
        std::string type = j.value("type", "Primitive");
        p.name        = j.value("name", "");
        p.texture     = j.value("texture", "");
        p.maxInstances= j.value("maxInstances", 128);
        p.billboard   = j.value("billboard", true);
        p.emitRate    = j.value("emitRate", 30.0f);
        p.autoEmit    = j.value("autoEmit", false);
        p.burstCount  = j.value("burstCount", 10);
        p.lifeMin     = j.value("lifeMin", 0.5f);
        p.lifeMax     = j.value("lifeMax", 1.5f);

        auto readV3 = [&](const char* key, Vector3 def)->Vector3 {
            if (!j.contains(key)) return def;
            auto arr = j[key];
            if (arr.size() != 3) return def;
            return { arr[0],arr[1],arr[2] };
        };
        auto readV4 = [&](const char* key, Vector4 def)->Vector4 {
            if (!j.contains(key)) return def;
            auto arr = j[key];
            if (arr.size() != 4) return def;
            return { arr[0],arr[1],arr[2],arr[3] };
        };

        p.posMin   = readV3("posMin", {});
        p.posMax   = readV3("posMax", {});
        p.velMin   = readV3("velMin", {});
        p.velMax   = readV3("velMax", {});
        p.scaleMin = readV3("scaleMin", {1,1,1});
        p.scaleMax = readV3("scaleMax", {1,1,1});
        p.colMin   = readV4("colMin", {1,1,1,1});
        p.colMax   = readV4("colMax", {1,1,1,1});

        if (type == "Ring") {
            CreateEmitter<RingEmitter>(p);
        } else if (type == "Cylinder") {
            CreateEmitter<CylinderEmitter>(p);
        } else if (type == "Original") {
            CreateEmitter<OriginalEmitter>(p);
        } else {
            CreateEmitter<PrimitiveEmitter>(p);
        }
    }
}