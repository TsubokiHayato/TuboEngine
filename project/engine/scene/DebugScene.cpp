#include "DebugScene.h"
#include "ParticleManager.h"
#include "PrimitiveEmitter.h"
#include "RingEmitter.h"
#include "CylinderEmitter.h"
#include "OriginalEmitter.h"
#include "BlendMode.h"
#include "ImGuiManager.h"
#include "ModelManager.h"
#include "SceneManager.h"
#include "TextureManager.h"
#include "Input.h"

void DebugScene::Initialize() {

	std::string testDDSTextureHandle = "rostock_laage_airport_4k.dds";

	// Audio
	const std::string audioFileName = "fanfare.wav";
	audio = std::make_unique<Audio>();
	audio->Initialize(audioFileName);

	// SkyBox
	skyBox = std::make_unique<SkyBox>();
	skyBox->Initialize(testDDSTextureHandle);

	// Camera
	camera = std::make_unique<Camera>();
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);

	// SceneChange
	sceneChangeAnimation = std::make_unique<SceneChangeAnimation>(1280, 720, 80, 1.5f, "barrier.png");
	sceneChangeAnimation->Initialize();

	// Emitters
	if (!particleInitialized_) {
		auto* pm = ParticleManager::GetInstance();

		{
			ParticlePreset p{};
			p.name = "Smoke";
			p.texture = "particle.png";
			p.autoEmit = false;
			p.emitRate = 45.0f;
			p.burstCount = 20;
			p.lifeMin = 0.8f; p.lifeMax = 1.6f;
			p.posMin = {-1,0,-1}; p.posMax = {1,0,1};
			p.velMin = {-0.5f,0.8f,-0.5f}; p.velMax = {0.5f,1.8f,0.5f};
			p.scaleStart = {0.4f,0.4f,0.4f}; p.scaleEnd = {0.9f,0.9f,0.9f};
			p.colorStart = {0.7f,0.7f,0.7f,0.6f}; p.colorEnd = {1,1,1,0.0f};
			pm->CreateEmitter<PrimitiveEmitter>(p);
			emitterNames_.push_back(p.name);
		}
		{
			ParticlePreset p{};
			p.name = "Ring";
			p.texture = "gradationLine.png";
			p.burstCount = 32;
			p.lifeMin = 0.6f; p.lifeMax = 1.2f;
			p.scaleStart = {0.8f,0.8f,1}; p.scaleEnd = {1.2f,1.2f,1};
			p.colorStart = {0.8f,0.6f,1.0f,0.9f}; p.colorEnd = {1.0f,0.9f,1.0f,0.0f};
			pm->CreateEmitter<RingEmitter>(p);
			emitterNames_.push_back(p.name);
		}
		{
			ParticlePreset p{};
			p.name = "Fountain";
			p.texture = "gradationLine.png";
			p.emitRate = 25.0f;
			p.lifeMin = 0.8f; p.lifeMax = 1.8f;
			p.posMin = {-0.5f,0,-0.5f}; p.posMax = {0.5f,0,0.5f};
			p.velMin = {-0.2f,1.5f,-0.2f}; p.velMax = {0.2f,2.5f,0.2f};
			p.scaleStart = {0.4f,0.4f,0.4f}; p.scaleEnd = {0.7f,0.7f,0.7f};
			p.colorStart = {0.6f,0.8f,1.0f,0.7f}; p.colorEnd = {0.9f,1.0f,1.0f,0.0f};
			pm->CreateEmitter<CylinderEmitter>(p);
			emitterNames_.push_back(p.name);
		}
		{
			ParticlePreset p{};
			p.name = "Radial";
			p.texture = "particle.png";
			p.burstCount = 40;
			p.lifeMin = 0.7f; p.lifeMax = 1.4f;
			p.velMin = {1,0,0}; p.velMax = {3,0,0};
			p.scaleStart = {0.4f,0.4f,0.4f}; p.scaleEnd = {0.2f,0.2f,0.2f};
			p.colorStart = {1.0f,0.8f,0.5f,0.7f}; p.colorEnd = {1.0f,1.0f,0.8f,0.0f};
			pm->CreateEmitter<OriginalEmitter>(p);
			emitterNames_.push_back(p.name);
		}

		particleInitialized_ = true;
	}
}

void DebugScene::Update() {

	// Camera
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);
	camera->Update();

	// SkyBox
	skyBox->SetCamera(camera.get());
	skyBox->Update();

	// Scene change
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		if (sceneChangeAnimation->IsFinished()) {
			sceneChangeAnimation->SetPhase(SceneChangeAnimation::Phase::Appearing);
			isRequestSceneChange = true;
		}
	}
	sceneChangeAnimation->Update(1.0f / 60.0f);
	if (isRequestSceneChange && sceneChangeAnimation->IsFinished()) {
		SceneManager::GetInstance()->ChangeScene(SCENE::TITLE);
		isRequestSceneChange = false;
	}

	// エミッター存在チェック（削除・Undo後のダングリング回避）
	auto* pm = ParticleManager::GetInstance();
	for (size_t i = 0; i < emitterNames_.size();) {
		if (!pm->Find(emitterNames_[i])) {
			emitterNames_.erase(emitterNames_.begin() + i);
		} else {
			++i;
		}
	}

	// Particles
	pm->Update(1.0f/60.0f, camera.get());

	// 操作
	if (Input::GetInstance()->TriggerKey(DIK_B)) {
		for (auto& name : emitterNames_) {
			if (auto* e = pm->Find(name)) e->Emit(e->GetPreset().burstCount);
		}
	}
	if (Input::GetInstance()->TriggerKey(DIK_1) && emitterNames_.size() >= 1) {
		if (auto* e = pm->Find(emitterNames_[0])) e->GetPreset().autoEmit = !e->GetPreset().autoEmit;
	}
	if (Input::GetInstance()->TriggerKey(DIK_2) && emitterNames_.size() >= 2) {
		if (auto* e = pm->Find(emitterNames_[1])) e->GetPreset().autoEmit = !e->GetPreset().autoEmit;
	}
	if (Input::GetInstance()->TriggerKey(DIK_3) && emitterNames_.size() >= 3) {
		if (auto* e = pm->Find(emitterNames_[2])) e->GetPreset().autoEmit = !e->GetPreset().autoEmit;
	}
	if (Input::GetInstance()->TriggerKey(DIK_4) && emitterNames_.size() >= 4) {
		if (auto* e = pm->Find(emitterNames_[3])) e->GetPreset().autoEmit = !e->GetPreset().autoEmit;
	}

	if (Input::GetInstance()->TriggerKey(DIK_R)) {
		for (auto& name : emitterNames_) if (auto* e = pm->Find(name)) e->ClearAll();
	}

	if (Input::GetInstance()->TriggerKey(DIK_S)) {
		pm->SaveAll("Resources/Particles/all.json");
	}
	if (Input::GetInstance()->TriggerKey(DIK_L)) {
		pm->LoadAll("Resources/Particles/all.json");
		// 再構築（重複名は自動リネームされるため Find で再取得）
		std::vector<std::string> newNames = {"Smoke","Ring","Fountain","Radial"};
		emitterNames_.clear();
		for (auto& nm : newNames) if (pm->Find(nm)) emitterNames_.push_back(nm);
	}
}

void DebugScene::Finalize() {}

void DebugScene::Object3DDraw() {
	// 必要に応じて
	// skyBox->Draw();
}

void DebugScene::SpriteDraw() {
	sceneChangeAnimation->Draw();
}

void DebugScene::ImGuiDraw() {
#ifdef USE_IMGUI
	ImGui::Begin("DebugScene");
	ImGui::Text("Particles Test");
	ImGui::BulletText("B: Burst all, R: Clear all");
	ImGui::BulletText("1/2/3/4: Toggle AutoEmit (Smoke/Ring/Fountain/Radial)");
	ImGui::BulletText("S: SaveAll, L: LoadAll (Resources/Particles/all.json)");
	ImGui::End();

#ifdef _DEBUG
	ImGui::Begin("Camera");
	ImGui::DragFloat3("Position", &cameraPosition.x, 0.1f);
	ImGui::DragFloat3("Rotation", &cameraRotation.x, 0.1f);
	ImGui::DragFloat3("Scale",    &cameraScale.x,    0.1f);
	ImGui::End();
#endif

	// 全エミッター管理 UI（保存/ロード・個別編集が可能）
	ParticleManager::GetInstance()->DrawImGui();

	sceneChangeAnimation->DrawImGui();
#endif // USE_IMGUI
}

void DebugScene::ParticleDraw() {
	ParticleManager::GetInstance()->Draw();
}
