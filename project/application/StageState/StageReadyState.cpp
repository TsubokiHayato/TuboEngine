#include "StageReadyState.h"
#include "LineManager.h"
#include "StageScene.h"
#include "StageType.h"


void StageReadyState::Enter(StageScene* scene) {
    // マップチップフィールド初期化
    scene->GetMapChipField()->LoadMapChipCsv(scene->GetMapChipCsvFilePath());

    // カメラ初期化
    if (scene->GetMainCamera()) {
        auto* map = scene->GetMapChipField();
        float centerX = (map->GetNumBlockHorizontal() * MapChipField::GetBlockWidth()) / 8.0f;
        float centerY = (map->GetNumBlockVirtical() * MapChipField::GetBlockHeight()) / 2.0f;
        scene->GetMainCamera()->SetTranslate({centerX, centerY, 70.0f});
        scene->GetMainCamera()->setRotation({3.14f, 0.0f, 0.0f});
        scene->GetMainCamera()->setScale({1.0f, 1.0f, 1.0f});
        scene->GetMainCamera()->Update();
    }

    // プレイヤーのマップチップ座標取得
    int playerMapX = -1, playerMapY = -1;
    for (uint32_t y = 0; y < scene->GetMapChipField()->GetNumBlockVirtical(); ++y) {
        for (uint32_t x = 0; x < scene->GetMapChipField()->GetNumBlockHorizontal(); ++x) {
            if (scene->GetMapChipField()->GetMapChipTypeByIndex(x, y) == MapChipType::Player) {
                playerMapX = x;
                playerMapY = y;
            }
        }
    }

    // プレイヤー初期化
    scene->GetPlayer()->Initialize();
    scene->GetPlayer()->SetCamera(scene->GetMainCamera());
    scene->GetPlayer()->SetMapChipField(scene->GetMapChipField());
    scene->GetPlayer()->Update();
    playerTargetPosition_ = scene->GetMapChipField()->GetMapChipPositionByIndex(playerMapX, playerMapY);

    // ブロック生成・レイヤー計算（チェビシェフ距離）
    scene->GetBlocks().clear();
    blockTargetPositions_.clear();
    blockRippleLayers_.clear();
    for (uint32_t y = 0; y < scene->GetMapChipField()->GetNumBlockVirtical(); ++y) {
        for (uint32_t x = 0; x < scene->GetMapChipField()->GetNumBlockHorizontal(); ++x) {
            if (scene->GetMapChipField()->GetMapChipTypeByIndex(x, y) == MapChipType::kBlock) {
                auto block = std::make_unique<Block>();
                Vector3 pos = scene->GetMapChipField()->GetMapChipPositionByIndex(x, y);
                block->Initialize(pos);
                block->SetCamera(scene->GetMainCamera());
                block->Update();
                scene->GetBlocks().push_back(std::move(block));
                blockTargetPositions_.push_back(pos);
                int layer = std::max(std::abs((int)x - playerMapX), std::abs((int)y - playerMapY));
                blockRippleLayers_.push_back((float)layer);
            }
        }
    }

    // エネミー生成・レイヤー計算（チェビシェフ距離）
    scene->GetEnemies().clear();
    enemyTargetPositions_.clear();
    enemyRippleLayers_.clear();
    for (uint32_t y = 0; y < scene->GetMapChipField()->GetNumBlockVirtical(); ++y) {
        for (uint32_t x = 0; x < scene->GetMapChipField()->GetNumBlockHorizontal(); ++x) {
            if (scene->GetMapChipField()->GetMapChipTypeByIndex(x, y) == MapChipType::Enemy) {
                auto enemy = std::make_unique<Enemy>();
                enemy->Initialize();
                enemy->SetCamera(scene->GetMainCamera());
                enemy->SetPlayer(scene->GetPlayer());
                Vector3 pos = scene->GetMapChipField()->GetMapChipPositionByIndex(x, y);
                enemy->SetPosition(pos);
                enemy->Update();
                scene->GetEnemies().push_back(std::move(enemy));
                enemyTargetPositions_.push_back(pos);
                int layer = std::max(std::abs((int)x - playerMapX), std::abs((int)y - playerMapY));
                enemyRippleLayers_.push_back((float)layer);
            }
        }
    }

    // アニメーション用タイマー初期化
    currentDroppingLayer_ = 0;
    layerDropTimer_ = 0.0f;
    isDropFinished_ = false;
    prevTime_ = std::chrono::steady_clock::now();
}

void StageReadyState::Update(StageScene* scene) {
    using namespace std::chrono;
    auto now = steady_clock::now();
    float deltaTime = duration_cast<duration<float>>(now - prevTime_).count();
    prevTime_ = now;

    LineManager::GetInstance()->SetDefaultCamera(scene->GetMainCamera());

    if (!isDropFinished_) {
        layerDropTimer_ += deltaTime;

        auto lerp = [](float a, float b, float t) { return a + (b - a) * t; };

        // ブロック
        auto& blocks = scene->GetBlocks();
        for (size_t i = 0; i < blocks.size(); ++i) {
            int layer = (int)blockRippleLayers_[i];
            if (layer < currentDroppingLayer_) {
                // すでに落下済み → 目標座標に固定
                blocks[i]->SetPosition(blockTargetPositions_[i]);
            } else if (layer == currentDroppingLayer_) {
                // 今落下中 → 補間
                float t = std::min(layerDropTimer_ / kDropDuration, 1.0f);
                const Vector3& target = blockTargetPositions_[i];
                Vector3 start = target;
                start.z += kDropOffsetZ;
                Vector3 pos;
                pos.x = lerp(start.x, target.x, t);
                pos.y = lerp(start.y, target.y, t);
                pos.z = lerp(start.z, target.z, t);
                blocks[i]->SetPosition(pos);
            } else {
                // まだ落下していない → 上空で待機
                Vector3 pos = blockTargetPositions_[i];
                pos.z += kDropOffsetZ;
                blocks[i]->SetPosition(pos);
            }
        }

        // プレイヤー
        if (currentDroppingLayer_ > 0) {
            // すでに落下済み
            scene->GetPlayer()->SetPosition(playerTargetPosition_);
        } else {
            // 今落下中
            float t = std::min(layerDropTimer_ / kDropDuration, 1.0f);
            Vector3 start = playerTargetPosition_;
            start.z += kDropOffsetZ;
            Vector3 pos;
            pos.x = lerp(start.x, playerTargetPosition_.x, t);
            pos.y = lerp(start.y, playerTargetPosition_.y, t);
            pos.z = lerp(start.z, playerTargetPosition_.z, t);
            scene->GetPlayer()->SetPosition(pos);
        }

        // エネミー
        auto& enemies = scene->GetEnemies();
        for (size_t i = 0; i < enemies.size(); ++i) {
            int layer = (int)enemyRippleLayers_[i];
            if (layer < currentDroppingLayer_) {
                enemies[i]->SetPosition(enemyTargetPositions_[i]);
            } else if (layer == currentDroppingLayer_) {
                float t = std::min(layerDropTimer_ / kDropDuration, 1.0f);
                const Vector3& target = enemyTargetPositions_[i];
                Vector3 start = target;
                start.z += kDropOffsetZ;
                Vector3 pos;
                pos.x = lerp(start.x, target.x, t);
                pos.y = lerp(start.y, target.y, t);
                pos.z = lerp(start.z, target.z, t);
                enemies[i]->SetPosition(pos);
            } else {
                Vector3 pos = enemyTargetPositions_[i];
                pos.z += kDropOffsetZ;
                enemies[i]->SetPosition(pos);
            }
        }

        // レイヤー内の全オブジェクトが落下完了したか判定
        bool layerFinished = true;
        // ブロック
        for (size_t i = 0; i < blocks.size(); ++i)
            if ((int)blockRippleLayers_[i] == currentDroppingLayer_ && layerDropTimer_ < kDropDuration)
                layerFinished = false;
        // エネミー
        for (size_t i = 0; i < enemies.size(); ++i)
            if ((int)enemyRippleLayers_[i] == currentDroppingLayer_ && layerDropTimer_ < kDropDuration)
                layerFinished = false;
        // プレイヤー
        if (currentDroppingLayer_ == 0 && layerDropTimer_ < kDropDuration)
            layerFinished = false;

        if (layerFinished) {
            // 次のレイヤーへ
            ++currentDroppingLayer_;
            layerDropTimer_ = 0.0f;
            // 最大レイヤーを超えたら終了
            int maxLayer = 0;
            for (auto l : blockRippleLayers_) maxLayer = std::max(maxLayer, (int)l);
            for (auto l : enemyRippleLayers_) maxLayer = std::max(maxLayer, (int)l);
            if (currentDroppingLayer_ > maxLayer)
                isDropFinished_ = true;
        }

        for (auto& block : scene->GetBlocks()) block->Update();
        scene->GetPlayer()->Update();
        for (auto& enemy : scene->GetEnemies()) enemy->Update();

        return;
    }

    // アニメーション終了後
    for (auto& block : scene->GetBlocks()) block->Update();
    scene->GetPlayer()->Update();
    for (auto& enemy : scene->GetEnemies()) enemy->Update();
    LineManager::GetInstance()->SetDefaultCamera(scene->GetFollowCamera()->GetCamera());
}

void StageReadyState::Exit(StageScene* scene) {}

void StageReadyState::Object3DDraw(StageScene* scene) {
    for (auto& block : scene->GetBlocks()) block->Draw();
    scene->GetPlayer()->Draw();
    for (auto& enemy : scene->GetEnemies()) enemy->Draw();
}

void StageReadyState::SpriteDraw(StageScene* scene) {}

void StageReadyState::ImGuiDraw(StageScene* scene) {
    if (scene->GetMainCamera()) {
        if (ImGui::CollapsingHeader("MainCamera")) {
            Vector3 camPos = scene->GetMainCamera()->GetTranslate();
            Vector3 camRot = scene->GetMainCamera()->GetRotation();
            Vector3 camScale = scene->GetMainCamera()->GetScale();

            if (ImGui::DragFloat3("Position", &camPos.x, 0.1f)) {
                scene->GetMainCamera()->SetTranslate(camPos);
                scene->GetMainCamera()->Update();
            }
            if (ImGui::DragFloat3("Rotation", &camRot.x, 0.1f)) {
                scene->GetMainCamera()->setRotation(camRot);
                scene->GetMainCamera()->Update();
            }
            if (ImGui::DragFloat3("Scale", &camScale.x, 0.1f)) {
                scene->GetMainCamera()->setScale(camScale);
                scene->GetMainCamera()->Update();
            }
        }
    }

    if (ImGui::CollapsingHeader("Drop Animation")) {
        float t = std::min(layerDropTimer_ / kDropDuration, 1.0f);
        ImGui::ProgressBar(t, ImVec2(0.0f, 0.0f), "Progress");

        ImGui::SameLine();
        if (ImGui::Button("Skip Animation")) {
            // 全て即座に完了
            int maxLayer = 0;
            for (auto l : blockRippleLayers_) maxLayer = std::max(maxLayer, (int)l);
            for (auto l : enemyRippleLayers_) maxLayer = std::max(maxLayer, (int)l);
            currentDroppingLayer_ = maxLayer + 1;
            isDropFinished_ = true;
        }
        ImGui::Text("isDropFinished: %s", isDropFinished_ ? "true" : "false");
    }
}

void StageReadyState::ParticleDraw(StageScene* scene) {}
