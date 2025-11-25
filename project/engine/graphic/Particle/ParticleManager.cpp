#include "ParticleManager.h"
#include "ImGuiManager.h"
#include "Camera.h"
#include <externals/nlohmann/json.hpp>
#include <fstream>
#include <algorithm>
#include <cstdarg>
#include "PrimitiveEmitter.h"
#include "RingEmitter.h"
#include "CylinderEmitter.h"
#include "OriginalEmitter.h"
#include "OrbitTrailEmitter.h" // 追加: 軌道トレイルエミッター


static void ApplyParticleManagerTheme(int themeId = 0) {
#ifdef USE_IMGUI
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding  = 4.0f;
    style.GrabRounding   = 4.0f;
    style.ScrollbarRounding = 6.0f;
    style.FramePadding = ImVec2(8, 5);
    style.ItemSpacing  = ImVec2(8, 6);
    style.WindowPadding= ImVec2(10, 10);
    style.TabRounding  = 4.0f;

    ImVec4* colors = style.Colors;
    if (themeId == 0) { // Dark
        colors[ImGuiCol_WindowBg]        = ImVec4(0.12f,0.12f,0.14f,1.0f);
        colors[ImGuiCol_ChildBg]         = ImVec4(0.10f,0.10f,0.12f,1.0f);
        colors[ImGuiCol_FrameBg]         = ImVec4(0.20f,0.22f,0.26f,1.0f);
        colors[ImGuiCol_FrameBgHovered]  = ImVec4(0.30f,0.34f,0.40f,1.0f);
        colors[ImGuiCol_FrameBgActive]   = ImVec4(0.36f,0.40f,0.48f,1.0f);
        colors[ImGuiCol_TitleBg]         = ImVec4(0.08f,0.08f,0.10f,1.0f);
        colors[ImGuiCol_TitleBgActive]   = ImVec4(0.16f,0.18f,0.22f,1.0f);
        colors[ImGuiCol_Button]          = ImVec4(0.25f,0.28f,0.34f,1.0f);
        colors[ImGuiCol_ButtonHovered]   = ImVec4(0.35f,0.40f,0.48f,1.0f);
        colors[ImGuiCol_ButtonActive]    = ImVec4(0.40f,0.46f,0.55f,1.0f);
        colors[ImGuiCol_Header]          = ImVec4(0.22f,0.26f,0.32f,1.0f);
        colors[ImGuiCol_HeaderHovered]   = ImVec4(0.32f,0.38f,0.46f,1.0f);
        colors[ImGuiCol_HeaderActive]    = ImVec4(0.38f,0.44f,0.52f,1.0f);
        colors[ImGuiCol_Tab]             = ImVec4(0.18f,0.20f,0.26f,1.0f);
        colors[ImGuiCol_TabHovered]      = ImVec4(0.34f,0.38f,0.46f,1.0f);
        colors[ImGuiCol_TabActive]       = ImVec4(0.28f,0.32f,0.40f,1.0f);
        colors[ImGuiCol_PlotHistogram]   = ImVec4(0.26f,0.70f,0.55f,1.0f);
        colors[ImGuiCol_SliderGrab]      = ImVec4(0.42f,0.60f,1.00f,1.0f);
        colors[ImGuiCol_SliderGrabActive]= ImVec4(0.58f,0.74f,1.00f,1.0f);
    } else if (themeId == 1) { // Light
        ImGui::StyleColorsLight();
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.20f,0.55f,0.35f,1.0f);
    } else { // HighContrast
        ImGui::StyleColorsDark();
        colors[ImGuiCol_WindowBg]      = ImVec4(0.05f,0.05f,0.05f,1.0f);
        colors[ImGuiCol_Button]        = ImVec4(0.30f,0.05f,0.05f,1.0f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.55f,0.15f,0.15f,1.0f);
        colors[ImGuiCol_ButtonActive]  = ImVec4(0.70f,0.20f,0.20f,1.0f);
        colors[ImGuiCol_Header]        = ImVec4(0.20f,0.25f,0.55f,1.0f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f,0.35f,0.75f,1.0f);
        colors[ImGuiCol_HeaderActive]  = ImVec4(0.40f,0.45f,0.85f,1.0f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.88f,0.70f,0.10f,1.0f);
    }
#endif // USE_IMGUI
}

namespace {
	inline void FixMinMax(float& mn, float& mx) {
		if (mn > mx) std::swap(mn, mx);
	}
	inline void FixMinMax(Vector3& mn, Vector3& mx) {
		FixMinMax(mn.x, mx.x); FixMinMax(mn.y, mx.y); FixMinMax(mn.z, mx.z);
	}
	inline void FixMinMax(Vector4& mn, Vector4& mx) {
		FixMinMax(mn.x, mx.x); FixMinMax(mn.y, mx.y); FixMinMax(mn.z, mx.z); FixMinMax(mn.w, mx.w);
	}
	inline void FixPresetRanges(ParticlePreset& p) {
		FixMinMax(p.lifeMin, p.lifeMax);
		FixMinMax(p.posMin, p.posMax);
		FixMinMax(p.velMin, p.velMax);
		FixMinMax(p.scaleMin, p.scaleMax);
		FixMinMax(p.colMin, p.colMax);
	}
}

ParticleManager::ParticleManager() {
	// 初期ロード (存在すれば)
	InitialLoad("Resources/Particles/all.json");
}

void ParticleManager::SetStatus(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vsnprintf(statusMsg_, sizeof(statusMsg_), fmt, args);
	va_end(args);
	statusTimer_ = 3.0f;
}

void ParticleManager::MarkChanged() {
	changedThisFrame_ = true;
}

std::string ParticleManager::GenerateUniqueName(const std::string& base) const {
	if (base.empty()) return GenerateUniqueName("Emitter");
	bool exists = false;
	for (auto& e : emitters_) {
		if (e->GetName() == base) { exists = true; break; }
	}
	if (!exists) return base;
	// "(n)" 付与
	int counter = 2;
	while (true) {
		std::string candidate = base + "(" + std::to_string(counter) + ")";
		bool dup = false;
		for (auto& e : emitters_) {
			if (e->GetName() == candidate) { dup = true; break; }
		}
		if (!dup) return candidate;
		++counter;
	}
}

void ParticleManager::Update(float dt, Camera* cam) {
	for (auto& e : emitters_) {
		auto& preset = e->GetPreset();
		if (preset.autoEmit && preset.emitRate > 0.0f) {
			float interval = 1.0f / preset.emitRate;
			preset._emitAccum += dt;
			while (preset._emitAccum >= interval) {
				e->Emit(1);
				preset._emitAccum -= interval;
			}
		}
		e->Update(dt, cam);
	}
	if (statusTimer_ > 0.0f) {
		statusTimer_ -= dt;
		if (statusTimer_ <= 0.0f) statusMsg_[0] = '\0';
	}
	if (changedThisFrame_) {
		CaptureHistory();
		changedThisFrame_ = false;
	}

	// プレビュー更新
	UpdatePreview(dt, cam);
}

void ParticleManager::Draw() {
	auto* cmd = DirectXCommon::GetInstance()->GetCommandList().Get();
	for (auto& e : emitters_) {
		e->Draw(cmd);
	}
	DrawPreview(cmd);
}

void ParticleManager::DrawStatusBar() {
#ifdef USE_IMGUI
	if (statusMsg_[0] != '\0') {
		ImVec4 col = ImVec4(0.2f,0.6f,0.9f,1.0f);
		ImGui::TextColored(col, "%s", statusMsg_);
	}
#endif
}

void ParticleManager::DrawTemplatesSection() {
#ifdef USE_IMGUI
	ImGui::Separator();
	ImGui::Text("Templates");
	static int tmplIndex = 0;
	const char* items = "Smoke\0RingBurst\0Fountain\0Radial\0OrbitTrail\0"; // 追加
	ImGui::Combo("Template", &tmplIndex, items);
	if (ImGui::Button("Add Template")) {
		ParticlePreset p{};
		switch (tmplIndex) {
		case 0: // Smoke
			p.name = "Smoke";
			p.texture = "particle.png";
			p.emitRate = 40.0f;
			p.burstCount = 20;
			p.lifeMin = 0.8f; p.lifeMax = 1.6f;
			p.posMin = {-1.0f,0.0f,-1.0f}; p.posMax = {1.0f,0.0f,1.0f};
			p.velMin = {-0.5f,0.8f,-0.5f}; p.velMax = {0.5f,1.5f,0.5f};
			p.scaleStart = {0.4f,0.4f,0.4f}; p.scaleEnd = {0.9f,0.9f,0.9f};
			p.colorStart = {0.7f,0.7f,0.7f,0.6f}; p.colorEnd = {1.0f,1.0f,1.0f,0.0f};
			CreateEmitter<PrimitiveEmitter>(p);
			break;
		case 1: // RingBurst
			p.name = "Ring";
			p.texture = "gradationLine.png";
			p.burstCount = 32;
			p.lifeMin = 0.6f; p.lifeMax = 1.2f;
			p.scaleStart = {0.8f,0.8f,1.0f}; p.scaleEnd = {1.2f,1.2f,1.0f};
			p.colorStart = {0.8f,0.6f,1.0f,0.9f}; p.colorEnd = {1.0f,0.9f,1.0f,0.0f};
			CreateEmitter<RingEmitter>(p);
			break;
		case 2: // Fountain
			p.name = "Fountain";
			p.texture = "gradationLine.png";
			p.emitRate = 25.0f;
			p.lifeMin = 0.8f; p.lifeMax = 1.8f;
			p.posMin = {-0.5f,0.0f,-0.5f}; p.posMax = {0.5f,0.0f,0.5f};
			p.velMin = {-0.2f,1.5f,-0.2f}; p.velMax = {0.2f,2.5f,0.2f};
			p.scaleStart = {0.4f,0.4f,0.4f}; p.scaleEnd = {0.7f,0.7f,0.7f};
			p.colorStart = {0.6f,0.8f,1.0f,0.7f}; p.colorEnd = {0.9f,1.0f,1.0f,0.0f};
			CreateEmitter<CylinderEmitter>(p);
			break;
		case 3: // Radial
			p.name = "Radial";
			p.texture = "particle.png";
			p.burstCount = 40;
			p.lifeMin = 0.7f; p.lifeMax = 1.4f;
			p.velMin = {1.0f,0.0f,0.0f}; p.velMax = {3.0f,0.0f,0.0f};
			p.scaleStart = {0.4f,0.4f,0.4f}; p.scaleEnd = {0.2f,0.2f,0.2f};
			p.colorStart = {1.0f,0.8f,0.5f,0.7f}; p.colorEnd = {1.0f,1.0f,0.8f,0.0f};
			CreateEmitter<OriginalEmitter>(p);
			break;
		case 4: // OrbitTrail (Player trail)
			p.name = "OrbitTrail";
			p.texture = "particle.png"; // 適宜変更
			p.emitRate = 60.0f;
			p.autoEmit = true;
			p.lifeMin = 0.4f; p.lifeMax = 0.8f;
			p.scaleStart = {0.15f,0.15f,0.15f}; p.scaleEnd = {0.05f,0.05f,0.05f};
			p.colorStart = {0.6f,0.8f,1.0f,0.9f}; p.colorEnd = {0.2f,0.4f,1.0f,0.0f};
			// radius と angularSpeed は OrbitTrailEmitter 内部でデフォルト利用 (必要なら後で SetRadius 等呼び出し)
			CreateEmitter<OrbitTrailEmitter>(p);
			break;
		}
	}
#endif
}

void ParticleManager::DrawEmittersSection() {
#ifdef USE_IMGUI
    ImGui::Separator();
    ImGui::Text("Emitters");

    // --- 新規生成 UI ---
    static int newType = 0;
    ImGui::Combo("New Type", &newType, "Primitive\0Ring\0Cylinder\0Original\0OrbitTrail\0"); // 追加
    static char nameBuf[64] = "Emitter";
    static char texBuf[128] = "particle.png";
    static ParticlePreset tmp{};
    ImGui::InputText("Name", nameBuf, sizeof(nameBuf));
    ImGui::InputText("Texture", texBuf, sizeof(texBuf));
    tmp.name = nameBuf;
    tmp.texture = texBuf;
    ImGui::Checkbox("AutoEmit", &tmp.autoEmit);
    ImGui::DragFloat("EmitRate", &tmp.emitRate, 0.2f, 0.0f, 300.0f);
    ImGui::DragInt("BurstCount", (int*)&tmp.burstCount, 1, 1, 2000);
    ImGui::DragInt("MaxInstances", (int*)&tmp.maxInstances, 1, 1, 200000);
    ImGui::DragFloat2("LifeRange", &tmp.lifeMin, 0.01f, 0.01f, 100.0f);
    ImGui::DragFloat3("PosMin", &tmp.posMin.x, 0.01f);
    ImGui::DragFloat3("PosMax", &tmp.posMax.x, 0.01f);
    ImGui::DragFloat3("VelMin", &tmp.velMin.x, 0.01f);
    ImGui::DragFloat3("VelMax", &tmp.velMax.x, 0.01f);
    ImGui::DragFloat3("ScaleStart", &tmp.scaleStart.x, 0.01f);
    ImGui::DragFloat3("ScaleEnd", &tmp.scaleEnd.x, 0.01f);
    ImGui::ColorEdit4("ColorStart", &tmp.colorStart.x);
    ImGui::ColorEdit4("ColorEnd", &tmp.colorEnd.x);
    if (ImGui::Button("Create")) {
        FixPresetRanges(tmp);
        switch (newType) {
        case 0: CreateEmitter<PrimitiveEmitter>(tmp); break;
        case 1: CreateEmitter<RingEmitter>(tmp); break;
        case 2: CreateEmitter<CylinderEmitter>(tmp); break;
        case 3: CreateEmitter<OriginalEmitter>(tmp); break;
        case 4: CreateEmitter<OrbitTrailEmitter>(tmp); break; // 追加
        }
    }
    ImGui::SameLine();
    ImGui::Checkbox("Live Preview", &previewEnabled_);
    ImGui::SameLine();
    if (ImGui::Button("Burst Preview") && previewEnabled_) {
        if (previewEmitter_)
            previewEmitter_->Emit(tmp.burstCount);
    }
    if (previewEnabled_) {
        ApplyPreviewPreset(tmp, newType);
        ImGui::TextDisabled("Preview running...");
        ImGui::SameLine();
        if (ImGui::Button("Reset Preview")) {
            if (previewEmitter_) {
                previewEmitter_->ClearAll();
                previewEmitter_->GetPreset()._emitAccum = 0.0f;
            }
        }
    } else {
        if (previewEmitter_) { previewEmitter_.reset(); previewType_ = -1; }
    }
    ImGui::Separator();
    // --- 保存 / 読込 ---
    if (ImGui::Button("Save Selected") && !selectedEmitter_.empty()) {
        SaveSelected("Resources/Particles/" + selectedEmitter_ + ".json", { selectedEmitter_ });
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Merge Selected") && !selectedEmitter_.empty()) {
        pendingAction_ = PendingActionType::LoadMergeSelected;
        pendingEmitterName_ = selectedEmitter_;
        OpenConfirmPopup("ConfirmAction", ("Load & Merge '" + pendingEmitterName_ + "' ?").c_str());
    }
    ImGui::SameLine();
    if (ImGui::Button("Save All") && !emitters_.empty()) { SaveAll("Resources/Particles/all.json"); }
    ImGui::SameLine();
    if (ImGui::Button("Load All")) {
        pendingAction_ = PendingActionType::LoadAll;
        OpenConfirmPopup("ConfirmAction", "Load All (overwrite current emitters)?");
    }

    // --- エミッター列挙 (必ず TreePop 対応) ---
    for (size_t i = 0; i < emitters_.size(); /*手動++*/ ) {
        auto& e = emitters_[i];
        auto& p = e->GetPreset();
        bool open = ImGui::TreeNode(p.name.c_str());
        if (!open) { ++i; continue; }

        selectedEmitter_ = p.name;
        uint32_t oldMax = p.maxInstances;

        ImGui::Text("Instances: %u / %u", p.maxInstances, p.maxInstances);
        ImGui::Checkbox(("AutoEmit##" + p.name).c_str(), &p.autoEmit);
        ImGui::DragFloat(("EmitRate##" + p.name).c_str(), &p.emitRate, 0.1f, 0.0f, 400.0f);
        ImGui::DragInt(("Burst##" + p.name).c_str(), (int*)&p.burstCount, 1, 1, 5000);
        ImGui::DragInt(("MaxInstances##" + p.name).c_str(), (int*)&p.maxInstances, 1, 1, 500000);

        bool lifeEd    = ImGui::DragFloat2(("LifeRange##" + p.name).c_str(), &p.lifeMin, 0.01f, 0.01f, 100.0f);
        bool posMinE   = ImGui::DragFloat3(("PosMin##" + p.name).c_str(), &p.posMin.x, 0.01f);
        bool posMaxE   = ImGui::DragFloat3(("PosMax##" + p.name).c_str(), &p.posMax.x, 0.01f);
        bool velMinE   = ImGui::DragFloat3(("VelMin##" + p.name).c_str(), &p.velMin.x, 0.01f);
        bool velMaxE   = ImGui::DragFloat3(("VelMax##" + p.name).c_str(), &p.velMax.x, 0.01f);
        bool sclStartE = ImGui::DragFloat3(("ScaleStart##" + p.name).c_str(), &p.scaleStart.x, 0.01f);
        bool sclEndE   = ImGui::DragFloat3(("ScaleEnd##" + p.name).c_str(), &p.scaleEnd.x, 0.01f);
        bool colStartE = ImGui::ColorEdit4(("ColorStart##" + p.name).c_str(), &p.colorStart.x);
        bool colEndE   = ImGui::ColorEdit4(("ColorEnd##" + p.name).c_str(), &p.colorEnd.x);
        bool gravE     = ImGui::DragFloat3(("Gravity##" + p.name).c_str(), &p.gravity.x, 0.01f);
        bool dragE     = ImGui::DragFloat(("Drag##" + p.name).c_str(), &p.drag, 0.001f, 0.0f, 2.0f);
        bool billboardE= ImGui::Checkbox(("Billboard##" + p.name).c_str(), &p.billboard);
        bool worldSpaceE=ImGui::Checkbox(("WorldSpace##" + p.name).c_str(), &p.simulateInWorldSpace);
        bool emitterPosE=ImGui::DragFloat3(("EmitterPos##" + p.name).c_str(), &p.emitterTransform.translate.x, 0.01f);

        if (lifeEd||posMinE||posMaxE||velMinE||velMaxE||sclStartE||sclEndE||
            colStartE||colEndE||gravE||dragE||billboardE||worldSpaceE||emitterPosE||
            oldMax != p.maxInstances) {
            FixPresetRanges(p);
            MarkChanged();
        }
        if (oldMax != p.maxInstances) {
            e->ReallocateInstanceBufferIfNeeded();
            SetStatus("Reallocated '%s' maxInstances=%u", p.name.c_str(), p.maxInstances);
        }

        if (ImGui::Button(("Emit Burst##" + p.name).c_str())) {
            e->Emit(p.burstCount);
            MarkChanged();
        }
        ImGui::SameLine();
        if (ImGui::Button(("Clear##" + p.name).c_str())) {
            pendingAction_ = PendingActionType::ClearEmitter;
            pendingEmitterName_ = p.name;
            OpenConfirmPopup("ConfirmAction", ("Clear particles in '" + p.name + "' ?").c_str());
        }
        ImGui::SameLine();
        if (ImGui::Button(("Delete##" + p.name).c_str())) {
            SetStatus("Removed '%s'", p.name.c_str());
            emitters_.erase(emitters_.begin() + i);
            if (selectedEmitter_ == p.name) selectedEmitter_.clear();
            MarkChanged();
            ImGui::TreePop();
            continue; // erase したので i はインクリメントせず次要素へ
        }

        ImGui::TreePop();
        ++i;
    }

    ImGui::Separator();
    if (ImGui::Button("Undo")) {
        pendingAction_ = PendingActionType::UndoAction;
        OpenConfirmPopup("ConfirmAction", "Undo last change?");
    }
    ImGui::SameLine();
    if (ImGui::Button("Redo")) {
        pendingAction_ = PendingActionType::RedoAction;
        OpenConfirmPopup("ConfirmAction", "Redo next change?");
    }

    if (ImGui::BeginPopupModal("ConfirmAction", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("%s", confirmMessage_.c_str());
        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(80, 0))) {
            ExecutePendingAction();
            ImGui::CloseCurrentPopup();
            pendingAction_ = PendingActionType::None;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(80, 0))) {
            pendingAction_ = PendingActionType::None;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
#endif
}

// DrawImGui 内に emitter 列挙ロジックが重複している場合は削除し、以下のみ残す
void ParticleManager::DrawImGui() {
#ifdef USE_IMGUI
    if (ImGui::Begin("Particle Manager")) {
        static int themeId = 0;
        ImGui::RadioButton("Dark", &themeId, 0); ImGui::SameLine();
        ImGui::RadioButton("Light", &themeId, 1); ImGui::SameLine();
        ImGui::RadioButton("HiContrast", &themeId, 2);
        static int lastTheme = -1;
        if (lastTheme != themeId) { ApplyParticleManagerTheme(themeId); lastTheme = themeId; }
        ImGui::Separator();

        DrawStatusBar();
        DrawEmittersSection();     
        DrawTemplatesSection();   
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

std::string ParticleManager::BuildSnapshotJson() const {
	nlohmann::json root;
	for (auto& e : emitters_) {
		const auto& p = e->GetPreset();
		std::string type = "Primitive";
		if (dynamic_cast<RingEmitter*>(e.get())) type = "Ring";
		else if (dynamic_cast<CylinderEmitter*>(e.get())) type = "Cylinder";
		else if (dynamic_cast<OriginalEmitter*>(e.get())) type = "Original";
		else if (dynamic_cast<OrbitTrailEmitter*>(e.get())) type = "OrbitTrail"; // 追加
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
			{"posMin",{p.posMin.x,p.posMin.y,p.posMin.z}},
			{"posMax",{p.posMax.x,p.posMax.y,p.posMax.z}},
			{"velMin",{p.velMin.x,p.velMin.y,p.velMin.z}},
			{"velMax",{p.velMax.x,p.velMax.y,p.velMax.z}},
			{"scaleStart",{p.scaleStart.x,p.scaleStart.y,p.scaleStart.z}},
			{"scaleEnd",{p.scaleEnd.x,p.scaleEnd.y,p.scaleEnd.z}},
			{"colorStart",{p.colorStart.x,p.colorStart.y,p.colorStart.z,p.colorStart.w}},
			{"colorEnd",{p.colorEnd.x,p.colorEnd.y,p.colorEnd.z,p.colorEnd.w}},
			{"gravity",{p.gravity.x,p.gravity.y,p.gravity.z}},
			{"drag",p.drag},
			{"center",{p.center.x,p.center.y,p.center.z}},
			{"simulateInWorldSpace",p.simulateInWorldSpace},
			{"emitterPos",{p.emitterTransform.translate.x,p.emitterTransform.translate.y,p.emitterTransform.translate.z}}
		};
		root["emitters"].push_back(j);
	}
	return root.dump();
}

void ParticleManager::CaptureHistory() {
	std::string snap = BuildSnapshotJson();
	// 現在Indexより後ろを切る（Undo後に変更されたとき）
	if (historyIndex_ + 1 < (int)history_.size()) {
		history_.erase(history_.begin() + historyIndex_ + 1, history_.end());
	}
	history_.push_back(snap);
	historyIndex_ = (int)history_.size() - 1;
	if (history_.size() > 50) { // 上限
		history_.erase(history_.begin());
		--historyIndex_;
	}
}

void ParticleManager::ApplySnapshot(const std::string& jsonStr) {
	nlohmann::json root;
	try {
		root = nlohmann::json::parse(jsonStr);
	} catch (...) {
		SetStatus("Snapshot parse error");
		return;
	}
	emitters_.clear();
	if (!root.contains("emitters")) {
		SetStatus("Snapshot missing 'emitters'");
		return;
	}
	for (auto& j : root["emitters"]) {
		ParticlePreset p;
		std::string type = j.value("type", "Primitive");
		p.name = j.value("name", "");
		p.texture = j.value("texture", "");
		p.maxInstances = j.value("maxInstances", 128);
		p.billboard = j.value("billboard", true);
		p.emitRate = j.value("emitRate", 30.0f);
		p.autoEmit = j.value("autoEmit", false);
		p.burstCount = j.value("burstCount", 10);
		p.lifeMin = j.value("lifeMin", 0.5f);
		p.lifeMax = j.value("lifeMax", 1.5f);
		auto readV3 = [&](const char* key, Vector3 def)->Vector3 {
			if (!j.contains(key)) return def;
			auto arr = j[key];
			if (arr.size() != 3) return def;
			return {arr[0],arr[1],arr[2]};
		};
		auto readV4 = [&](const char* key, Vector4 def)->Vector4 {
			if (!j.contains(key)) return def;
			auto arr = j[key];
			if (arr.size() != 4) return def;
			return {arr[0],arr[1],arr[2],arr[3]};
		};
		p.posMin = readV3("posMin", {});
		p.posMax = readV3("posMax", {});
		p.velMin = readV3("velMin", {});
		p.velMax = readV3("velMax", {});
		p.scaleStart = readV3("scaleStart", {1,1,1});
		p.scaleEnd = readV3("scaleEnd", {1,1,1});
		p.colorStart = readV4("colorStart", {1,1,1,1});
		p.colorEnd = readV4("colorEnd", {1,1,1,0});
		p.gravity = readV3("gravity", {0,-0.5f,0});
		p.center = readV3("center", {0,0,0});
		Vector3 emPos = readV3("emitterPos", {0,0,0});
		p.emitterTransform.translate = emPos;
		p.drag = j.value("drag", 0.0f);
		p.simulateInWorldSpace = j.value("simulateInWorldSpace", true);

		if (type == "Ring") {
			CreateEmitter<RingEmitter>(p);
		} else if (type == "Cylinder") {
			CreateEmitter<CylinderEmitter>(p);
		} else if (type == "Original") {
			CreateEmitter<OriginalEmitter>(p);
		} else if (type == "OrbitTrail") {
			CreateEmitter<OrbitTrailEmitter>(p); // 追加
		} else {
			CreateEmitter<PrimitiveEmitter>(p);
		}
	}
	SetStatus("Snapshot applied (%zu emitters)", emitters_.size());
}

void ParticleManager::Undo() {
	if (historyIndex_ <= 0 || history_.empty()) {
		SetStatus("Undo unavailable");
		return;
	}
	--historyIndex_;
	ApplySnapshot(history_[historyIndex_]);
}

void ParticleManager::Redo() {
	if (historyIndex_ + 1 >= (int)history_.size()) {
		SetStatus("Redo unavailable");
		return;
	}
	++historyIndex_;
	ApplySnapshot(history_[historyIndex_]);
}

void ParticleManager::SaveAll(const std::string& path) {
	nlohmann::json root;
	root["version"] = 2;
	root["snapshot"] = nlohmann::json::parse(BuildSnapshotJson());
	std::ofstream ofs(path);
	if (!ofs) {
		SetStatus("Save failed: %s", path.c_str());
		return;
	}
	ofs << root.dump(2);
	SetStatus("Saved %zu emitters", emitters_.size());
}

void ParticleManager::LoadAll(const std::string& path) {
	std::ifstream ifs(path);
	if (!ifs) {
		SetStatus("Load failed: %s", path.c_str());
		return;
	}
	nlohmann::json root;
	try {
		root = nlohmann::json::parse(ifs);
	} catch (...) {
		SetStatus("Load parse error");
		return;
	}
	emitters_.clear();
	nlohmann::json snap;
	if (root.contains("snapshot")) snap = root["snapshot"];
	else if (root.contains("emitters")) snap = root; // 旧形式互換
	else {
		SetStatus("Load: missing snapshot");
		return;
	}
	if (!snap.contains("emitters")) {
		SetStatus("Load: no emitters field");
		return;
	}
	for (auto& j : snap["emitters"]) {
		ParticlePreset p;
		std::string type = j.value("type", "Primitive");
		p.name = j.value("name", "");
		p.texture = j.value("texture", "");
		p.maxInstances = j.value("maxInstances", 128);
		p.billboard = j.value("billboard", true);
		p.emitRate = j.value("emitRate", 30.0f);
		p.autoEmit = j.value("autoEmit", false);
		p.burstCount = j.value("burstCount", 10);
		p.lifeMin = j.value("lifeMin", 0.5f);
		p.lifeMax = j.value("lifeMax", 1.5f);
		auto readV3 = [&](const char* key, Vector3 def)->Vector3 {
			if (!j.contains(key)) return def;
			auto arr = j[key];
			if (arr.size() != 3) return def;
			return {arr[0],arr[1],arr[2]};
		};
		auto readV4 = [&](const char* key, Vector4 def)->Vector4 {
			if (!j.contains(key)) return def;
			auto arr = j[key];
			if (arr.size() != 4) return def;
			return {arr[0],arr[1],arr[2],arr[3]};
		};
		p.posMin = readV3("posMin", {});
		p.posMax = readV3("posMax", {});
		p.velMin = readV3("velMin", {});
		p.velMax = readV3("velMax", {});
		p.scaleStart = readV3("scaleStart", {1,1,1});
		p.scaleEnd = readV3("scaleEnd", {1,1,1});
		p.colorStart = readV4("colorStart", {1,1,1,1});
		p.colorEnd = readV4("colorEnd", {1,1,1,0});
		p.gravity = readV3("gravity", {0,-0.5f,0});
		p.center = readV3("center", {0,0,0});
		Vector3 emPos = readV3("emitterPos", {0,0,0});
		p.emitterTransform.translate = emPos;
		p.drag = j.value("drag", 0.0f);
		p.simulateInWorldSpace = j.value("simulateInWorldSpace", true);

		if (type == "Ring") {
			CreateEmitter<RingEmitter>(p);
		} else if (type == "Cylinder") {
			CreateEmitter<CylinderEmitter>(p);
		} else if (type == "Original") {
			CreateEmitter<OriginalEmitter>(p);
		} else if (type == "OrbitTrail") {
			CreateEmitter<OrbitTrailEmitter>(p); // 追加
		} else {
			CreateEmitter<PrimitiveEmitter>(p);
		}
	}
	SetStatus("Loaded %zu emitters", emitters_.size());
	MarkChanged();
}

void ParticleManager::SaveSelected(const std::string& path, const std::vector<std::string>& names) {
	nlohmann::json root;
	for (auto& name : names) {
		auto* e = Find(name);
		if (!e) continue;
		const auto& p = e->GetPreset();
		std::string type = "Primitive";
		if (dynamic_cast<RingEmitter*>(e)) type = "Ring";
		else if (dynamic_cast<CylinderEmitter*>(e)) type = "Cylinder";
		else if (dynamic_cast<OriginalEmitter*>(e)) type = "Original";
		else if (dynamic_cast<OrbitTrailEmitter*>(e)) type = "OrbitTrail"; // 追加
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
			{"posMin",{p.posMin.x,p.posMin.y,p.posMin.z}},
			{"posMax",{p.posMax.x,p.posMax.y,p.posMax.z}},
			{"velMin",{p.velMin.x,p.velMin.y,p.velMin.z}},
			{"velMax",{p.velMax.x,p.velMax.y,p.velMax.z}},
			{"scaleStart",{p.scaleStart.x,p.scaleStart.y,p.scaleStart.z}},
			{"scaleEnd",{p.scaleEnd.x,p.scaleEnd.y,p.scaleEnd.z}},
			{"colorStart",{p.colorStart.x,p.colorStart.y,p.colorStart.z,p.colorStart.w}},
			{"colorEnd",{p.colorEnd.x,p.colorEnd.y,p.colorEnd.z,p.colorEnd.w}},
			{"gravity",{p.gravity.x,p.gravity.y,p.gravity.z}},
			{"drag",p.drag},
			{"simulateInWorldSpace",p.simulateInWorldSpace},
			{"emitterPos",{p.emitterTransform.translate.x,p.emitterTransform.translate.y,p.emitterTransform.translate.z}}
		};
		root["emitters"].push_back(j);
	}
	std::ofstream ofs(path);
	if (!ofs) {
		SetStatus("SaveSelected failed");
		return;
	}
	ofs << root.dump(2);
	SetStatus("Saved %zu selected", root["emitters"].size());
}

void ParticleManager::LoadMerge(const std::string& path) {
	std::ifstream ifs(path);
	if (!ifs) {
		SetStatus("Merge load failed");
		return;
	}
	nlohmann::json root;
	try { root = nlohmann::json::parse(ifs); }
	catch (...) {
		SetStatus("Merge parse error");
		return;
	}
	if (!root.contains("emitters")) {
		SetStatus("Merge: no emitters");
		return;
	}
	size_t added = 0;
	for (auto& j : root["emitters"]) {
		ParticlePreset p;
		std::string type = j.value("type", "Primitive");
		p.name = j.value("name", "");
		p.texture = j.value("texture", "");
		p.maxInstances = j.value("maxInstances", 128);
		p.billboard = j.value("billboard", true);
		p.emitRate = j.value("emitRate", 30.0f);
		p.autoEmit = j.value("autoEmit", false);
		p.burstCount = j.value("burstCount", 10);
		p.lifeMin = j.value("lifeMin", 0.5f);
		p.lifeMax = j.value("lifeMax", 1.5f);
		auto readV3 = [&](const char* key, Vector3 def)->Vector3 {
			if (!j.contains(key)) return def;
			auto arr = j[key];
			if (arr.size() != 3) return def;
			return {arr[0],arr[1],arr[2]};
		};
		p.posMin = readV3("posMin", {});
		p.posMax = readV3("posMax", {});
		p.velMin = readV3("velMin", {});
		p.velMax = readV3("velMax", {});
		p.scaleStart = readV3("scaleStart", {1,1,1});
		p.scaleEnd = readV3("scaleEnd", {1,1,1});
		p.gravity = readV3("gravity", {0,-0.5f,0});
		Vector3 emPos = readV3("emitterPos", {0,0,0});
		p.emitterTransform.translate = emPos;

		if (type == "Ring") CreateEmitter<RingEmitter>(p);
		else if (type == "Cylinder") CreateEmitter<CylinderEmitter>(p);
		else if (type == "Original") CreateEmitter<OriginalEmitter>(p);
		else if (type == "OrbitTrail") CreateEmitter<OrbitTrailEmitter>(p); // 追加
		else CreateEmitter<PrimitiveEmitter>(p);
		++added;
	}
	SetStatus("Merged %zu emitters", added);
	MarkChanged();
}

void ParticleManager::InitialLoad(const std::string& filePath) {
	if (initialLoaded_) return;
	initialLoaded_ = true;
	std::ifstream ifs(filePath);
	if (!ifs) {
		SetStatus("Initial load skipped (%s not found)", filePath.c_str());
		return;
	}
	LoadAll(filePath);
}

#ifdef USE_IMGUI
void ParticleManager::OpenConfirmPopup(const char* popupName, const char* message) {
    confirmMessage_ = message ? message : "";
    ImGui::OpenPopup(popupName);
}

void ParticleManager::ExecutePendingAction() {
    switch (pendingAction_) {
    case PendingActionType::DeleteEmitter:
        if (!pendingEmitterName_.empty()) {
            Remove(pendingEmitterName_);
            if (selectedEmitter_ == pendingEmitterName_) {
                selectedEmitter_.clear(); // 選択解除
            }
            SetStatus("Removed '%s'", pendingEmitterName_.c_str());
            MarkChanged();
        }
        break;
    case PendingActionType::ClearEmitter:
        if (auto* e = Find(pendingEmitterName_)) {
            e->ClearAll();
            SetStatus("Cleared '%s'", pendingEmitterName_.c_str());
            MarkChanged();
        }
        break;
    case PendingActionType::LoadAll:
        LoadAll("Resources/Particles/all.json");
        selectedEmitter_.clear();
        break;
    case PendingActionType::LoadMergeSelected:
        if (!pendingEmitterName_.empty()) {
            LoadMerge("Resources/Particles/" + pendingEmitterName_ + ".json");
        }
        break;
    case PendingActionType::UndoAction:
        Undo();
        break;
    case PendingActionType::RedoAction:
        Redo();
        break;
    case PendingActionType::None:
        break;
    }

    pendingEmitterName_.clear();
    pendingAction_ = PendingActionType::None;
}
#endif


// プレビュー適用ヘルパー
void ParticleManager::ApplyPreviewPreset(const ParticlePreset& src, int type) {
	if (!previewEnabled_)
		return;

	// 型変更や Texture 変更などで再生成が必要か判定
	if (previewEmitter_ == nullptr || previewType_ != type || previewCached_.texture != src.texture || previewCached_.billboard != src.billboard) {
		previewNeedsRecreate_ = true;
	}

	// プリセット差分（主な数値）の検出 (EmitRate などで即時更新したい)
	previewCached_ = src;

	if (previewNeedsRecreate_) {
		previewEmitter_.reset();
		switch (type) {
		case 0:
			previewEmitter_ = std::make_unique<PrimitiveEmitter>();
			break;
		case 1:
			previewEmitter_ = std::make_unique<RingEmitter>();
			break;
		case 2:
			previewEmitter_ = std::make_unique<CylinderEmitter>();
			break;
		case 3:
			previewEmitter_ = std::make_unique<OriginalEmitter>();
			break;
		case 4:
			previewEmitter_ = std::make_unique<OrbitTrailEmitter>();
			break; // 追加
		default:
			previewEmitter_ = std::make_unique<PrimitiveEmitter>();
			break;
		}
		previewType_ = type;

		// コピーして初期化
		ParticlePreset initPreset = src;
		initPreset.name = "_Preview"; // 固定名
		previewEmitter_->Initialize(initPreset);
		previewNeedsRecreate_ = false;
	} else {
		// 既存のプリセットを上書き
		if (previewEmitter_) {
			auto& dst = previewEmitter_->GetPreset();
			dst = src;
			dst.name = "_Preview";
		}
	}
}

// プレビュー更新 (Emit のシミュレーション)
void ParticleManager::UpdatePreview(float dt, Camera* cam) {
	if (!previewEnabled_ || !previewEmitter_)
		return;

	auto& p = previewEmitter_->GetPreset();
	if (p.autoEmit && p.emitRate > 0.0f) {
		float interval = 1.0f / p.emitRate;
		p._emitAccum += dt;
		while (p._emitAccum >= interval) {
			previewEmitter_->Emit(1);
			p._emitAccum -= interval;
		}
	}
	previewEmitter_->Update(dt, cam);
}

// プレビュー描画
void ParticleManager::DrawPreview(ID3D12GraphicsCommandList* cmd) {
	if (!previewEnabled_ || !previewEmitter_)
		return;
	previewEmitter_->Draw(cmd);
}
