#pragma once
#include "Bullet/BaseBullet.h"
#include "Object3d.h"
#include "Vector3.h"
#include <random>

class Player;

/// <summary>
/// 敵（CircusEnemy）が発射する追尾弾。旋回・蛇行パラメータを持ち、プレイヤーを追いかけるように飛ぶ。
/// </summary>
class CircusBullet : public BaseBullet {
public:
    /// <summary>
    /// 初期化処理。
    /// </summary>
    void Initialize(const TuboEngine::Math::Vector3& startPos) override;
    /// <summary>
    /// 更新処理。
    /// </summary>
    void Update() override;
    /// <summary>
    /// 描画処理。
    /// </summary>
    void Draw() override;
    /// <summary>
    /// 衝突時の処理。
    /// </summary>
    void OnCollision(Collider* other) override;
    /// <summary>
    /// 中心座標を取得する。
    /// </summary>
    TuboEngine::Math::Vector3 GetCenterPosition() const override;

    /// <summary>
    /// プレイヤーの参照を設定する。
    /// </summary>
    void SetPlayer(Player* player) { player_ = player; }
    /// <summary>
    /// カメラを設定する。
    /// </summary>
    void SetCamera(TuboEngine::Camera* camera);
    /// <summary>
    /// InitialVelocity を設定する。
    /// </summary>
    void SetInitialVelocity(const TuboEngine::Math::Vector3& vel) { velocity = vel; }
    /// <summary>
    /// TargetOffset を設定する。
    /// </summary>
    void SetTargetOffset(const TuboEngine::Math::Vector3& offset) { targetOffset_ = offset; }
    /// <summary>
    /// 速度を設定する。
    /// </summary>
    void SetSpeed(float speed) { speed_ = speed; }
    /// <summary>
    /// TurnSpeed を設定する。
    /// </summary>
    void SetTurnSpeed(float turnSpeed) { turnSpeed_ = turnSpeed; }
    /// <summary>
    /// SwerveAmplitude を設定する。
    /// </summary>
    void SetSwerveAmplitude(float amp) { swerveAmplitude_ = amp; }
    /// <summary>
    /// SwerveFrequency を設定する。
    /// </summary>
    void SetSwerveFrequency(float freq) { swerveFrequency_ = freq; }
    /// <summary>
    /// Phase1Duration を設定する。
    /// </summary>
    void SetPhase1Duration(float duration) { phase1Duration_ = duration; }
    /// <summary>
    /// TargetDelayFrames を設定する。
    /// </summary>
    void SetTargetDelayFrames(int delayFrames) { targetDelayFrames_ = delayFrames; }

    /// <summary>
    /// 生存フラグの取得・設定。
    /// </summary>
    bool GetIsAlive() const { return isAlive; }
    void SetIsAlive(bool alive) { isAlive = alive; }
    /// <summary>
    /// 座標を取得する。
    /// </summary>
    TuboEngine::Math::Vector3 GetPosition() const { return position; }

private:
    Player* player_ = nullptr;
    TuboEngine::Camera* camera_ = nullptr;
    
    float speed_ = 0.6f;
    float turnSpeed_ = 0.08f;
    float swerveAmplitude_ = 0.4f;
    float swerveFrequency_ = 5.0f;
    float phase1Duration_ = 0.5f;
    float elapsedTime_ = 0.0f;
    int targetDelayFrames_ = 25;

    TuboEngine::Math::Vector3 swerveOffset_{0, 0, 0};
    TuboEngine::Math::Vector3 targetOffset_{0, 0, 0};
    TuboEngine::Math::Vector3 lastTrailPos_{0, 0, 0}; // トレイルを隙間なく繋ぐための前回エミット位置
    class IParticleEmitter* trailEmitter_ = nullptr;
    class IParticleEmitter* explosionEmitter_ = nullptr;
    class IParticleEmitter* burnerEmitter_ = nullptr;
    class IParticleEmitter* sparkEmitter_ = nullptr; // 火花エミッター

    std::mt19937 rng_{std::random_device{}()};
};
