#pragma once
#include "StageState/IStageState.h"
#include "Vector3.h"
#include <chrono>
#include <vector>
#include <memory>
#include "Sprite.h"

class StageScene;

/// StageClear状態 （ステージクリア後の状態）///
class StageClearState : public IStageState {
public:
    /// <summary>
    /// ステートに入ったときの初期化処理。
    /// </summary>
    void Enter(StageScene* scene) override;
    /// <summary>
    /// 更新処理。
    /// </summary>
    void Update(StageScene* scene) override;
    /// <summary>
    /// ステートを抜けるときの後処理。
    /// </summary>
    void Exit(StageScene* scene) override;
    /// <summary>
    /// 3Dオブジェクト描画。
    /// </summary>
    void Object3DDraw(StageScene* scene) override;
    /// <summary>
    /// スプライト描画。
    /// </summary>
    void SpriteDraw(StageScene* scene) override;
    /// <summary>
    /// ImGui描画。
    /// </summary>
    void ImGuiDraw(StageScene* scene) override;
    /// <summary>
    /// パーティクル描画。
    /// </summary>
    void ParticleDraw(StageScene* scene) override;

private:
   
    // クリア後自動遷移用タイマー
    bool started_ = false;
    float timer_ = 0.0f;
    float durationSec_ = 1.2f; // この秒数後にシーンチェンジ演出を開始
    std::chrono::steady_clock::time_point lastFrameTime_;
};
