#include "ClearScene.h"
#include "Character/Player/Player.h"
#include "Object3d.h"
#include "TextManager.h"
#include "TextObject.h"
#include "engine/graphic/Particle/Effects/Primitive/PrimitiveEmitter.h"
#include "engine/graphic/Particle/ParticleManager.h"

#include <cmath>
#include <format>
#include "TextureManager.h"
#include "WinApp.h"
#include "externals/imgui/imgui.h"
#include"Animation/SceneChangeAnimation.h"
#include "Input.h"
#include "SceneManager.h" // 追加：シーン遷移呼び出し用
#include <random>         // 追加：ランダム変化に使用
#include <algorithm>
#undef max
#undef min

static float Clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }

// イージング関数（軽いイーズアウト）
static float EaseOutQuad(float t) { return 1.0f - (1.0f - t) * (1.0f - t); }
// 補助 Lerp
static TuboEngine::Math::Vector3 LerpVec(const TuboEngine::Math::Vector3& a, const TuboEngine::Math::Vector3& b, float t) {
    return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t };
}

// 浮動小数点のLerp
static float LerpF(float a, float b, float t) { return a + (b - a) * t; }

// 文字のクリア演出の初期化（廃止）

void ClearScene::Initialize() {
    // カメラの初期化
	camera = std::make_unique<TuboEngine::Camera>();
    cameraTransform.rotate = {0.0f, 0.0f, 0.0f};
    cameraTransform.scale = {1.0f, 1.0f, 1.0f};
    cameraTransform.translate = {0.0f, 0.0f, -15.0f};
    camera->SetTranslate(cameraTransform.translate);
    camera->setRotation(cameraTransform.rotate);
    camera->setScale(cameraTransform.scale);

    // プレイヤー初期化
    player_ = std::make_unique<Player>();
    player_->Initialize();
    // シーン側から操作するため入力は無効化（自動演出のみ）
    player_->SetMovementLocked(true);
    // カメラをセット（描画用）
    player_->SetCamera(camera.get());
    // 初期位置を手前に配置して見やすく
    player_->SetPosition({0.0f, 0.0f, 4.0f});
    player_->SetScale({1.0f, 1.0f, 1.0f});

    // クリアシーンでは入力が無効なので、マウス追従のRotate()が実行されず
    // Player::Initialize()の固定回転のままになりやすい。見た目が破綻しないよう
    // 「カメラ正面（画面奥方向）」を向く回転を明示的に指定しておく。
    // 本プロジェクトの移動/弾方向は rotation.z を基準にしているため z のみ整える。
    {
		TuboEngine::Math::Vector3 r = player_->GetRotation();
        r.x = 0.0f;
        r.y = 0.0f;
        r.z = 0.0f;
        player_->SetRotation(r);
    }

    player_->Update();

    // =============== 新演出：王冠モデルの初期化 ===============
    crownModel_ = std::make_unique<TuboEngine::Object3d>();
    crownModel_->Initialize("crown/crown.obj");
    // 初期位置: はるか上空
    crownModel_->SetPosition({0.0f, 25.0f, 4.0f}); 
    crownModel_->SetScale({1.0f, 1.0f, 1.0f}); // サイズは後で調整可能にする
    crownModel_->SetRotation({0.0f, 0.0f, 0.0f}); // Xは寝ないように調整が必要かも
    crownModel_->SetCamera(camera.get());
    crownModel_->SetModelColor({1.0f, 1.0f, 1.0f, 1.0f});
    crownModel_->Update();

    isCrowned_ = false;

    // TextManagerの初期化と「C L E A R」テキストの生成
    TuboEngine::TextManager::GetInstance()->Initialize();
    
    // ベースとなるフォント名を指定
    std::string fontName = TuboEngine::TextManager::PresetFontNames::Best10;
    
    // 実際に描画するサイズでフォントを作成
    float fontSize = 96.0f; 
    TuboEngine::TextManager::GetInstance()->GetOrCreateFontSized(fontName, fontSize);
    
	float screenW = static_cast<float>(TuboEngine::WinApp::GetInstance()->GetClientWidth());
	float screenH = static_cast<float>(TuboEngine::WinApp::GetInstance()->GetClientHeight());

    // C L E A R を生成（初期は完全透明）
    textClear_ = TuboEngine::TextManager::GetInstance()->CreateText(
        fontName + "_" + std::to_string(static_cast<int>(fontSize)), 
        "C L E A R", 
        {screenW * 0.5f, screenH * 0.25f}, 
        {1.0f, 1.0f, 1.0f, 0.0f}, 
        1.0f
    );
    
    if (textClear_) {
        textClear_->SetHorizontalAlign(1); // センタリング
        textClear_->SetVerticalAlign(1);   // 真ん中揃え
    }
    
    textFadeInTimer_ = 0.0f;

    // リスタート生成
    TuboEngine::TextManager::GetInstance()->GetOrCreateFontSized(fontName, 32.0f);
    textRestart_ = TuboEngine::TextManager::GetInstance()->CreateText(
        fontName + "_32", 
        "PRESS SPACE TO TITLE", 
        {screenW * 0.5f, screenH * 0.85f}, 
        {1.0f, 1.0f, 1.0f, 0.0f}, 
        1.0f
    );
    if (textRestart_) {
        textRestart_->SetHorizontalAlign(1);
        textRestart_->SetVerticalAlign(1);
    }
    textRestartBlinkTimer_ = 0.0f;

    // お祝い用紙吹雪パーティクルの準備
    hasEmittedConfetti_ = false;

    // 金色の紙吹雪
    ParticlePreset pGold{};
    pGold.name = "ConfettiGold";
    pGold.texture = "Hp.png";
    pGold.maxInstances = 200;
    pGold.autoEmit = false;
    pGold.burstCount = 150;
    pGold.lifeMin = 2.0f;
    pGold.lifeMax = 4.0f;
    pGold.posMin = {-2.0f, 0.0f, -2.0f};
    pGold.posMax = { 2.0f, 1.0f,  2.0f};
    pGold.velMin = {-15.0f, 10.0f, -15.0f};
    pGold.velMax = { 15.0f, 25.0f,  15.0f};
    pGold.gravity = {0.0f, -15.0f, 0.0f};
    pGold.drag = 0.02f;
    pGold.rotSpeedRangeZ = {-10.0f, 10.0f};
    pGold.scaleStart = {0.4f, 0.4f, 0.4f};
    pGold.scaleEnd = {0.1f, 0.1f, 0.1f};
    pGold.colorStart = {1.0f, 0.8f, 0.1f, 1.0f}; 
    pGold.colorEnd = {1.0f, 1.0f, 0.5f, 0.0f};
    
    // ピンク・オレンジ系の紙吹雪
    ParticlePreset pColor{};
    pColor.name = "ConfettiColor";
    pColor.texture = "hp.png";
    pColor.maxInstances = 200;
    pColor.autoEmit = false;
    pColor.burstCount = 150;
    pColor.lifeMin = 2.0f;
    pColor.lifeMax = 4.0f;
    pColor.posMin = {-2.0f, 0.0f, -2.0f};
    pColor.posMax = { 2.0f, 1.0f,  2.0f};
    pColor.velMin = {-18.0f, 15.0f, -18.0f};
    pColor.velMax = { 18.0f, 30.0f,  18.0f};
    pColor.gravity = {0.0f, -12.0f, 0.0f};
    pColor.drag = 0.03f;
    pColor.rotSpeedRangeZ = {-10.0f, 10.0f};
    pColor.scaleStart = {0.3f, 0.3f, 0.3f};
    pColor.scaleEnd = {0.05f, 0.05f, 0.05f};
    pColor.colorStart = {1.0f, 0.2f, 0.6f, 1.0f}; 
    pColor.colorEnd = {1.0f, 0.5f, 0.8f, 0.0f};
    
    confettiGold_ = TuboEngine::ParticleManager::GetInstance()->CreateEmitter<PrimitiveEmitter>(pGold);
    confettiColor_ = TuboEngine::ParticleManager::GetInstance()->CreateEmitter<PrimitiveEmitter>(pColor);

    // シーンチェンジアニメーション初期化
    sceneChangeAnimation_ = std::make_unique<SceneChangeAnimation>(1280, 720, 80, 1.5f, "barrier.png");
    sceneChangeAnimation_->Initialize();

    isRequestSceneChange_ = false;
    elapsed_ = 0.0f;
    entranceTimer_ = 0.0f;
    entranceActive_ = true;
}

void ClearScene::Update() {
    const float delta = 1.0f / 60.0f; 
    elapsed_ += delta;

    // シーンチェンジ終了待ち
    if (!entranceActive_) {
        bool canStartEntrance =
            !delayEntranceUntilSceneChangeDone_ ||
            !sceneChangeAnimation_ ||
            sceneChangeAnimation_->IsFinished();
        if (canStartEntrance) {
            entranceActive_ = true;
            entranceTimer_ = 0.0f;
        }
    } else {
        entranceTimer_ += delta;
    }

    // プレイヤーの更新（ずっとプカプカ揺れるだけ）
    if (player_) {
        // Player::Update() 内部で Rotate() が走る可能性があるため、ここで固定値を入れる。
        {
			TuboEngine::Math::Vector3 fixedRot = player_->GetRotation();
            fixedRot.z = 0.0f;
            player_->SetRotation(fixedRot);
        }

        TuboEngine::Math::Vector3 pos = player_->GetPosition();
		TuboEngine::Math::Vector3 rot = player_->GetRotation();
	    TuboEngine::Math::Vector3 scl = player_->GetScale();

        // ベースのゆらぎ（常時）
        const float bobAmp = 0.6f;
        const float bobSpeed = 1.8f;
        pos.y = std::sin(elapsed_ * bobSpeed) * bobAmp;

        // XやZを固定（カメラ正面）
        pos.x = 0.0f;
        pos.z = 4.0f;
        
        rot.z = 0.12f * std::sin(elapsed_ * 1.5f);
        
        player_->SetPosition(pos);
        player_->SetRotation(rot);
        player_->Update();
    }

    // 王冠（Crown）のアニメーション更新
    if (crownModel_ && entranceActive_) {
        TuboEngine::Math::Vector3 crownPos = crownModel_->GetPosition();
        TuboEngine::Math::Vector3 crownRot = crownModel_->GetRotation();
        
        TuboEngine::Math::Vector3 targetHeadPos = {0.0f, 1.8f, 4.0f}; 
        float pRotZ = 0.0f;
        
        if (player_) {
            TuboEngine::Math::Vector3 pPos = player_->GetPosition();
            pRotZ = player_->GetRotation().z;
            
            // プレイヤーのZ軸回転（ゆらゆら）を考慮して頭の位置を計算
            float headOffset = 1.8f; // 頭の高さ
            float headX = -std::sin(pRotZ) * headOffset;
            float headY = std::cos(pRotZ) * headOffset;
            
            targetHeadPos.x = pPos.x + headX;
            targetHeadPos.y = pPos.y + headY;
            targetHeadPos.z = pPos.z;
        }

        if (!isCrowned_) {
            // クルクル回りながら降りてくる
            crownRot.y += 3.0f * delta; // Y軸回転
            crownRot.z = 0.0f;
            
            // 降下（Y軸のみ）
            crownPos.y -= 12.0f * delta;

            if (crownPos.y <= targetHeadPos.y) {
                isCrowned_ = true;
                crownPos = targetHeadPos;
                
                // 装着完了（Phase 3: パーティクル爆発 トリガー）
                if (!hasEmittedConfetti_) {
                    if (confettiGold_) {
                        confettiGold_->GetPreset().center = targetHeadPos;
                        confettiGold_->Emit(150);
                    }
                    if (confettiColor_) {
                        confettiColor_->GetPreset().center = targetHeadPos;
                        confettiColor_->Emit(150);
                    }
                    hasEmittedConfetti_ = true;
                }
            }
        } else {
            // 装着後はプレイヤーの頭上を追随し、ゆらゆらに合わせる
            crownPos = targetHeadPos;
            crownRot.z = pRotZ; // プレイヤーと同じ角度に傾ける
            crownRot.y += 1.0f * delta; 
            
            // テキストのフェードイン処理とアニメーション
            if (textClear_) {
                textFadeInTimer_ += delta;
                float alpha = textFadeInTimer_ / 0.5f; // 0.5秒かけてフェードイン
                if (alpha > 1.0f) alpha = 1.0f;
                // 色（アルファ値）を適用
                TuboEngine::Math::Vector4 currentColor = textClear_->GetColor();
                currentColor.w = alpha;
                textClear_->SetColor(currentColor);

                // プカプカ浮くアニメーション＆スケール
                float screenW = static_cast<float>(TuboEngine::WinApp::GetInstance()->GetClientWidth());
                float screenH = static_cast<float>(TuboEngine::WinApp::GetInstance()->GetClientHeight());
                float bounceY = std::sin(textFadeInTimer_ * 3.0f) * 15.0f; // 上下に揺れる
                float scaleAnim = 1.0f + std::sin(textFadeInTimer_ * 2.0f) * 0.05f; // 伸縮
                
                textClear_->SetPosition({screenW * 0.5f, screenH * 0.25f + bounceY});
                textClear_->SetScale(scaleAnim);
            }

            // リスタートの点滅処理
            if (textRestart_) {
                if (textFadeInTimer_ > 1.5f) { // CLEAR表示から少し待って開始
                    textRestartBlinkTimer_ += delta;
                    float restartAlpha = (std::sin(textRestartBlinkTimer_ * 4.0f) + 1.0f) * 0.5f; 
                    TuboEngine::Math::Vector4 rc = textRestart_->GetColor();
                    rc.w = restartAlpha * 0.8f; // 最大0.8程度
                    textRestart_->SetColor(rc);
                }
            }
        }

        crownModel_->SetPosition(crownPos);
        crownModel_->SetRotation(crownRot);
        crownModel_->Update();

        // テスト用：スペースでシーン遷移（王冠実装後すぐに帰れるように）
        if (isCrowned_ && TuboEngine::Input::GetInstance()->PushKey(DIK_SPACE)) {
            if (sceneChangeAnimation_ && sceneChangeAnimation_->IsFinished()) {
                sceneChangeAnimation_->SetPhase(SceneChangeAnimation::Phase::Appearing);
                isRequestSceneChange_ = true;
            }
        }
    }

    // カメラ更新
    camera->SetTranslate(cameraTransform.translate);
    camera->setRotation(cameraTransform.rotate);
    camera->setScale(cameraTransform.scale);
    camera->Update();

    // SceneChangeAnimation 更新
    if (sceneChangeAnimation_) {
        sceneChangeAnimation_->Update(delta);
    }

    // シーンチェンジアニメ完了でシーン遷移
    if (isRequestSceneChange_ && sceneChangeAnimation_ && sceneChangeAnimation_->IsFinished()) {
        SceneManager::GetInstance()->ChangeScene(SCENE::TITLE);
        isRequestSceneChange_ = false;
    }

    // TextManagerの更新
    TuboEngine::TextManager::GetInstance()->UpdateAll();

    // パーティクルの更新
    TuboEngine::ParticleManager::GetInstance()->Update(delta, camera.get());
}

void ClearScene::Finalize() {
    // 現在のシーンのTextObjectを破棄してメモリリークを防ぐ
    if (textClear_) {
        TuboEngine::TextManager::GetInstance()->RemoveText(textClear_);
        textClear_ = nullptr;
    }
    if (textRestart_) {
        TuboEngine::TextManager::GetInstance()->RemoveText(textRestart_);
        textRestart_ = nullptr;
    }
}

void ClearScene::Object3DDraw() {
    if (player_) player_->Draw();
    if (crownModel_) crownModel_->Draw();
}

void ClearScene::SpriteDraw() {
    // TextManagerでの文字描画
    TuboEngine::TextManager::GetInstance()->DrawAll();

    // シーンチェンジアニメーション描画
    if (sceneChangeAnimation_) {
        sceneChangeAnimation_->Draw();
    }
}

void ClearScene::ImGuiDraw() {

#ifdef USE_IMGUI
    ImGui::Begin("ClearScene");
    ImGui::Text("Clear Scene");
    ImGui::Text("Elapsed: %.2f", elapsed_);
    ImGui::End();

    // TextManagerのImGuiも表示しておくことで直感的にフォント追加や編集が可能に
    TuboEngine::TextManager::GetInstance()->DrawImGui();

    ImGui::Begin("Camera");
    ImGui::DragFloat3("Camera Translate", &cameraTransform.translate.x, 0.1f);
    ImGui::DragFloat3("Camera Rotate", &cameraTransform.rotate.x, 0.01f);
    ImGui::DragFloat3("Camera Scale", &cameraTransform.scale.x, 0.01f);
    ImGui::End();

    ImGui::Begin("Entrance Control");

    // 入場演出の制御
    ImGui::Checkbox("Delay Entrance Until SceneChange Done", &delayEntranceUntilSceneChangeDone_);
    if (ImGui::Button("Restart Entrance")) {
        entranceActive_ = (!delayEntranceUntilSceneChangeDone_) ||
                              (!sceneChangeAnimation_ || sceneChangeAnimation_->IsFinished());
        entranceTimer_ = 0.0f;
        isCrowned_ = false;
        
        // 王冠リセット
        if(crownModel_) {
            crownModel_->SetPosition({0.0f, 25.0f, 4.0f});
            crownModel_->SetRotation({0.0f, 0.0f, 0.0f});
        }
        
        // テキストリセット
        textFadeInTimer_ = 0.0f;
        if (textClear_) {
            TuboEngine::Math::Vector4 c = textClear_->GetColor();
            c.w = 0.0f;
            textClear_->SetColor(c);
        }
    }

    ImGui::Separator();
    if (crownModel_) {
        TuboEngine::Math::Vector3 cpos = crownModel_->GetPosition();
        TuboEngine::Math::Vector3 cscl = crownModel_->GetScale();
        TuboEngine::Math::Vector3 crot = crownModel_->GetRotation();
        ImGui::Text("Crown Transform");
        if (ImGui::DragFloat3("Pos", &cpos.x, 0.1f)) crownModel_->SetPosition(cpos);
        if (ImGui::DragFloat3("Scl", &cscl.x, 0.01f)) crownModel_->SetScale(cscl);
        if (ImGui::DragFloat3("Rot", &crot.x, 0.01f)) crownModel_->SetRotation(crot);
    }
    ImGui::End();

    // プレイヤー情報表示
    if (player_) {
        ImGui::Begin("Player (ClearScene)");
		TuboEngine::Math::Vector3 ppos = player_->GetPosition();
		TuboEngine::Math::Vector3 prot = player_->GetRotation();
		TuboEngine::Math::Vector3 pscale = player_->GetScale();
        ImGui::Text("Position: %.2f, %.2f, %.2f", ppos.x, ppos.y, ppos.z);
        ImGui::Text("Rotation: %.2f, %.2f, %.2f", prot.x, prot.y, prot.z);
        ImGui::Text("Scale: %.2f, %.2f, %.2f", pscale.x, pscale.y, pscale.z);
        ImGui::Separator();

        player_->DrawImGui();
        ImGui::End();
    }

#endif // USE_IMGUI
}

void ClearScene::ParticleDraw() {
    TuboEngine::ParticleManager::GetInstance()->Draw();
}
