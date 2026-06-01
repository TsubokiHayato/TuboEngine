#include "DebugScene.h"
#include "ParticleManager.h"
#include "Effects/Primitive/PrimitiveEmitter.h"
#include "Effects/Ring/RingEmitter.h"
#include "Effects/Cylinder/CylinderEmitter.h"
#include "Effects/Original/OriginalEmitter.h"
#include "BlendMode.h"
#include "ImGuiManager.h"
#include "ModelManager.h"
#include "SceneManager.h"
#include "TextureManager.h"
#include "Input.h"
#include "LineManager.h"
#include "TextManager.h"

void DebugScene::Initialize() {

    std::string testDDSTextureHandle = "rostock_laage_airport_4k.dds";

    // Camera
	camera = std::make_unique<TuboEngine::Camera>();
    camera->SetTranslate(cameraPosition);
    camera->setRotation(cameraRotation);
    camera->setScale(cameraScale);

   
    // TextManager。
    TuboEngine::TextManager* textManager = TuboEngine::TextManager::GetInstance();
 
    // Emitters
    if (!particleInitialized_) {
        TuboEngine::ParticleManager* pm = TuboEngine::ParticleManager::GetInstance();

        {
            ParticlePreset p{};
            p.name = "Smoke";
            p.texture = "particle.png";
            p.autoEmit = false;
            p.emitRate = 45.0f;
            p.burstCount = 20;
            p.lifeMin = 0.8f; p.lifeMax = 1.6f;
            p.posMin = TuboEngine::Math::Vector3{-1, 0, -1};
            p.posMax = TuboEngine::Math::Vector3{ 1, 0,  1};
            p.velMin = TuboEngine::Math::Vector3{-0.5f, 0.8f, -0.5f};
            p.velMax = TuboEngine::Math::Vector3{ 0.5f, 1.8f,  0.5f};
            p.scaleStart = TuboEngine::Math::Vector3{0.4f, 0.4f, 0.4f};
            p.scaleEnd   = TuboEngine::Math::Vector3{0.9f, 0.9f, 0.9f};
            p.colorStart = TuboEngine::Math::Vector4{0.7f, 0.7f, 0.7f, 0.6f};
            p.colorEnd   = TuboEngine::Math::Vector4{1.0f, 1.0f, 1.0f, 0.0f};
            pm->CreateEmitter<PrimitiveEmitter>(p);
            emitterNames_.push_back(p.name);
        }
        {
            ParticlePreset p{};
            p.name = "Ring";
            p.texture = "gradationLine.png";
            p.burstCount = 32;
            p.lifeMin = 0.6f; p.lifeMax = 1.2f;
            p.scaleStart = TuboEngine::Math::Vector3{0.8f, 0.8f, 1.0f};
            p.scaleEnd   = TuboEngine::Math::Vector3{1.2f, 1.2f, 1.0f};
            p.colorStart = TuboEngine::Math::Vector4{0.8f, 0.6f, 1.0f, 0.9f};
            p.colorEnd   = TuboEngine::Math::Vector4{1.0f, 0.9f, 1.0f, 0.0f};
            pm->CreateEmitter<RingEmitter>(p);
            emitterNames_.push_back(p.name);
        }
        {
            ParticlePreset p{};
            p.name = "Fountain";
            p.texture = "gradationLine.png";
            p.emitRate = 25.0f;
            p.lifeMin = 0.8f; p.lifeMax = 1.8f;
            p.posMin = TuboEngine::Math::Vector3{-0.5f, 0.0f, -0.5f};
            p.posMax = TuboEngine::Math::Vector3{ 0.5f, 0.0f,  0.5f};
            p.velMin = TuboEngine::Math::Vector3{-0.2f, 1.5f, -0.2f};
            p.velMax = TuboEngine::Math::Vector3{ 0.2f, 2.5f,  0.2f};
            p.scaleStart = TuboEngine::Math::Vector3{0.4f, 0.4f, 0.4f};
            p.scaleEnd   = TuboEngine::Math::Vector3{0.7f, 0.7f, 0.7f};
            p.colorStart = TuboEngine::Math::Vector4{0.6f, 0.8f, 1.0f, 0.7f};
            p.colorEnd   = TuboEngine::Math::Vector4{0.9f, 1.0f, 1.0f, 0.0f};
            pm->CreateEmitter<CylinderEmitter>(p);
            emitterNames_.push_back(p.name);
        }
        {
            ParticlePreset p{};
            p.name = "Radial";
            p.texture = "particle.png";
            p.burstCount = 40;
            p.lifeMin = 0.7f; p.lifeMax = 1.4f;
            p.velMin = TuboEngine::Math::Vector3{1.0f, 0.0f, 0.0f};
            p.velMax = TuboEngine::Math::Vector3{3.0f, 0.0f, 0.0f};
            p.scaleStart = TuboEngine::Math::Vector3{0.4f, 0.4f, 0.4f};
            p.scaleEnd   = TuboEngine::Math::Vector3{0.2f, 0.2f, 0.2f};
            p.colorStart = TuboEngine::Math::Vector4{1.0f, 0.8f, 0.5f, 0.7f};
            p.colorEnd   = TuboEngine::Math::Vector4{1.0f, 1.0f, 0.8f, 0.0f};
            pm->CreateEmitter<OriginalEmitter>(p);
            emitterNames_.push_back(p.name);
        }

        particleInitialized_ = true;
    }

    // SPH シミュレーター初期化
    if (!sphSimulator_) {
        sphSimulator_ = std::make_unique<SphSimulator>();
        // SphSimulator::Params のデフォルト値をそのまま使用
        // パラメータ調整は SphSimulator.h か ImGui で行う
        sphSimulator_->Initialize({}, camera.get(),
                                   "sphere/sphere.obj");
    }
}

void DebugScene::Update() {

    // Camera
    camera->SetTranslate(cameraPosition);
    camera->setRotation(cameraRotation);
    camera->setScale(cameraScale);
    camera->Update();

    
    // エミッター存在チェック
    TuboEngine::ParticleManager* pm = TuboEngine::ParticleManager::GetInstance();
    for (size_t i = 0; i < emitterNames_.size();) {
        if (!pm->Find(emitterNames_[i])) {
            emitterNames_.erase(emitterNames_.begin() + static_cast<std::ptrdiff_t>(i));
        } else {
            ++i;
        }
    }

    // Particles
    pm->Update(1.0f / 60.0f, camera.get());

    // SPH 更新 (GPU Compute + InstancedMeshRenderer 更新)
    if (sphSimulator_) {
        sphSimulator_->Update(1.0f / 60.0f, camera.get());
    }

	TuboEngine::LineManager::GetInstance()->SetDefaultCamera(camera.get());

    // TextManager Update
    TuboEngine::TextManager::GetInstance()->UpdateAll();
}

void DebugScene::Finalize() {
    if (testText_) {
        TuboEngine::TextManager::GetInstance()->RemoveText(testText_);
        testText_ = nullptr;
    }
    if (sphSimulator_) {
        sphSimulator_->Finalize();
        sphSimulator_.reset();
    }
}

void DebugScene::Object3DDraw() {
    // SPH: Object3d パイプラインでインスタンシング描画 (1 DrawCall)
    if (sphSimulator_) {
        sphSimulator_->Draw();
        sphSimulator_->DrawBounds({0.3f, 0.8f, 1.0f, 1.0f});
    }

    TuboEngine::LineManager::GetInstance()->DrawGrid(16.0f, 8, TuboEngine::Math::Vector3{}, TuboEngine::Math::Vector4{1.0f, 1.0f, 1.0f, 1.0f});
}

void DebugScene::SpriteDraw() {
   
    
    // TextManager Draw
    TuboEngine::TextManager::GetInstance()->DrawAll();
}

void DebugScene::ImGuiDraw() {

#ifdef USE_IMGUI
    ImGui::Begin("Camera");
    ImGui::DragFloat3("Position", &cameraPosition.x, 0.1f);
    ImGui::DragFloat3("Rotation", &cameraRotation.x, 0.1f);
    ImGui::DragFloat3("Scale",    &cameraScale.x,    0.1f);
    ImGui::End();

    // テキスト関連の ImGui は TextManager 側に任せる
    TuboEngine::TextManager::GetInstance()->DrawImGui();
#endif

    // パーティクル管理 UI とシーンチェンジ UI
    TuboEngine::ParticleManager::GetInstance()->DrawImGui();
    
    // SPH ImGui
    if (sphSimulator_) {
        sphSimulator_->DrawImGui();
    }
}

void DebugScene::ParticleDraw() {
    TuboEngine::ParticleManager::GetInstance()->Draw();
}
