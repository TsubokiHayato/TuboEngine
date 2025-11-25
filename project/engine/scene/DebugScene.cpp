#include "DebugScene.h"
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

		// 1) Smoke (Primitive)
		{
			ParticlePreset p{};
			p.name = "Smoke";
			p.texture = "particle.png";
			p.autoEmit = false;
			p.emitRate = 45.0f;
			p.burstCount = 20;
			p.lifeMin = 0.8f; p.lifeMax = 1.6f;
			p.posMin = {-1.0f, 0.0f, -1.0f}; p.posMax = { 1.0f, 0.0f, 1.0f};
			p.velMin = {-0.5f, 0.8f, -0.5f}; p.velMax = { 0.5f, 1.8f, 0.5f};
			p.scaleMin = {0.4f,0.4f,0.4f}; p.scaleMax = {0.9f,0.9f,0.9f};
			p.colMin = {0.7f,0.7f,0.7f,0.2f}; p.colMax = {1.0f,1.0f,1.0f,0.6f};
			emitters_.push_back(pm->CreateEmitter<PrimitiveEmitter>(p));
		}
		// 2) Ring burst
		{
			ParticlePreset p{};
			p.name = "Ring";
			p.texture = "gradationLine.png";
			p.autoEmit = false;
			p.burstCount = 32;
			p.lifeMin = 0.6f; p.lifeMax = 1.2f;
			p.scaleMin = {0.8f,0.8f,1.0f}; p.scaleMax = {1.2f,1.2f,1.0f};
			p.colMin = {0.8f,0.6f,1.0f,0.9f}; p.colMax = {1.0f,0.9f,1.0f,0.9f};
			emitters_.push_back(pm->CreateEmitter<RingEmitter>(p));
		}
		// 3) Fountain (Cylinder)
		{
			ParticlePreset p{};
			p.name = "Fountain";
			p.texture = "gradationLine.png";
			p.autoEmit = false;
			p.emitRate = 25.0f;
			p.lifeMin = 0.8f; p.lifeMax = 1.8f;
			p.posMin = {-0.5f, 0.0f, -0.5f}; p.posMax = {0.5f, 0.0f, 0.5f};
			p.velMin = {-0.2f, 1.5f, -0.2f}; p.velMax = {0.2f, 2.5f, 0.2f};
			p.colMin = {0.6f,0.8f,1.0f,0.7f}; p.colMax = {0.9f,1.0f,1.0f,0.9f};
			emitters_.push_back(pm->CreateEmitter<CylinderEmitter>(p));
		}
		// 4) Radial (Original)
		{
			ParticlePreset p{};
			p.name = "Radial";
			p.texture = "particle.png";
			p.autoEmit = false;
			p.burstCount = 40;
			p.lifeMin = 0.7f; p.lifeMax = 1.4f;
			p.velMin = {1.0f,0.0f,0.0f}; // 速度範囲に x を使用（OriginalEmitter 実装に準拠）
			p.velMax = {3.0f,0.0f,0.0f};
			p.colMin = {1.0f,0.8f,0.5f,0.7f}; p.colMax = {1.0f,1.0f,0.8f,0.9f};
			emitters_.push_back(pm->CreateEmitter<OriginalEmitter>(p));
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

	// Particles
	ParticleManager::GetInstance()->Update(1.0f / 60.0f, camera.get());

	// 操作
	// Burst 全部
	if (Input::GetInstance()->TriggerKey(DIK_B)) {
		for (auto* e : emitters_) if (e) e->Emit(e->GetPreset().burstCount);
	}
	// 1/2/3/4: AutoEmit 切り替え
	if (Input::GetInstance()->TriggerKey(DIK_1) && emitters_.size() >= 1) emitters_[0]->GetPreset().autoEmit = !emitters_[0]->GetPreset().autoEmit;
	if (Input::GetInstance()->TriggerKey(DIK_2) && emitters_.size() >= 2) emitters_[1]->GetPreset().autoEmit = !emitters_[1]->GetPreset().autoEmit;
	if (Input::GetInstance()->TriggerKey(DIK_3) && emitters_.size() >= 3) emitters_[2]->GetPreset().autoEmit = !emitters_[2]->GetPreset().autoEmit;
	if (Input::GetInstance()->TriggerKey(DIK_4) && emitters_.size() >= 4) emitters_[3]->GetPreset().autoEmit = !emitters_[3]->GetPreset().autoEmit;

	// R: 全クリア（ClearAll は IParticleEmitter に追加済み想定）
	if (Input::GetInstance()->TriggerKey(DIK_R)) {
		for (auto* e : emitters_) if (e) e->ClearAll();
	}

	// S/L: 全保存/読込
	if (Input::GetInstance()->TriggerKey(DIK_S)) {
		ParticleManager::GetInstance()->SaveAll("Resources/Particles/all.json");
	}
	if (Input::GetInstance()->TriggerKey(DIK_L)) {
		ParticleManager::GetInstance()->LoadAll("Resources/Particles/all.json");
		// ポインタ再取得
		emitters_.clear();
		auto* pm = ParticleManager::GetInstance();
		if (auto* e = pm->Find("Smoke")) emitters_.push_back(e);
		if (auto* e = pm->Find("Ring")) emitters_.push_back(e);
		if (auto* e = pm->Find("Fountain")) emitters_.push_back(e);
		if (auto* e = pm->Find("Radial")) emitters_.push_back(e);
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
