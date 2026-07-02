#pragma once
#include "Enemy.h"
#include "Bullet/Enemy/CircusBullet.h"
#include <vector>
#include <memory>
#include <random>

/// <summary>
/// 追尾弾（CircusBullet）を発射するタイプの敵。ジャスト回避時の弾消去にも対応する。
/// </summary>
class CircusEnemy : public Enemy {
public:
    /// <summary>
    /// コンストラクタ。
    /// </summary>
    CircusEnemy() = default;
    /// <summary>
    /// デストラクタ。
    /// </summary>
    ~CircusEnemy() override = default;

    /// <summary>
    /// 初期化処理。
    /// </summary>
    void Initialize() override;
    /// <summary>
    /// 更新処理。
    /// </summary>
    void Update() override;
    /// <summary>
    /// 描画処理。
    /// </summary>
    void Draw() override;
    /// <summary>
    /// スプライト描画。
    /// </summary>
    void DrawSprite();
    /// <summary>
    /// ImGuiによるデバッグ表示。
    /// </summary>
    void DrawImGui();

    // ジャスト回避時の同期・消去用
    float GetBulletSpeed() const { return bulletSpeed_; }
    float GetBulletTurnSpeed() const { return bulletTurnSpeed_; }
    float GetBulletSwerveAmp() const { return bulletSwerveAmp_; }
    float GetBulletSwerveFreq() const { return bulletSwerveFreq_; }
    float GetBulletPhase1Duration() const { return bulletPhase1Duration_; }
    
    /// <summary>
    /// 指定座標付近の弾を消去する。
    /// </summary>
    void ClearBulletsNear(const TuboEngine::Math::Vector3& center, float radius);
    void ClearAllBullets(); // ジャスト回避時に画面上の全弾を消去する

private:
    /// <summary>
    /// 通常弾を使用するかどうか。
    /// </summary>
    bool UseNormalBullet() const override { return false; }

    /// <summary>
    /// ミサイル発射を試みる。
    /// </summary>
    void TryFireMissiles(bool canSeePlayer, float dt);
    /// <summary>
    /// ミサイルを1発発射する。
    /// </summary>
    void FireSingleMissile(const TuboEngine::Math::Vector3& launchDir, float speed);

    bool isCharging_ = false;
    class IParticleEmitter* chargeEmitter_ = nullptr;
    class IParticleEmitter* muzzleFlashEmitter_ = nullptr;
    class IParticleEmitter* auraEmitter_ = nullptr;
    class IParticleEmitter* explosionEmitter_ = nullptr; // 死亡時の連鎖大爆発用

    // 死亡演出用
    float deathTimer_ = 0.0f;
    float nextExplosionTime_ = 0.0f;
    bool isDying_ = false; 

    std::unique_ptr<TuboEngine::Sprite> bossHpFrameSprite_;
    std::unique_ptr<TuboEngine::Sprite> bossHpBarSprite_;
    std::unique_ptr<TuboEngine::Sprite> bossHpBarBgSprite_; // 遅延用の背景バー

    /// <summary>
    /// HPバーUI用の簡易パーティクル。
    /// </summary>
    struct SimpleUIParticle {
        std::unique_ptr<TuboEngine::Sprite> sprite;
        TuboEngine::Math::Vector2 position;
        TuboEngine::Math::Vector2 velocity;
        float life = 0.0f;
        float maxLife = 1.0f;
        TuboEngine::Math::Vector4 color;
        bool active = false;
    };
    std::vector<SimpleUIParticle> hpParticles_;
    /// <summary>
    /// HpParticle を発生させる。
    /// </summary>
    void EmitHpParticle(const TuboEngine::Math::Vector2& pos);
    /// <summary>
    /// HpParticles の更新。
    /// </summary>
    void UpdateHpParticles(float dt);
    /// <summary>
    /// HpParticles の描画。
    /// </summary>
    void DrawHpParticles();

    float animatedHp_ = 350.0f; // Initialize()で上書きされるが念のため
    float hpParticleTimer_ = 0.0f;

    // ImGuiでの微調整用パラメータ
    TuboEngine::Math::Vector2 hpFrameOffset_ = {0.0f, 0.0f};
    TuboEngine::Math::Vector2 hpBarOffset_ = {0.0f, 0.0f};
    TuboEngine::Math::Vector2 hpBarSizeBase_ = {600.0f, 32.0f};
    TuboEngine::Math::Vector2 hpFrameSizeBase_ = {600.0f, 32.0f};

private:
    enum class AttackType {
        Burst,      // 一斉射撃
        Spiral,     // 回転しながら発射
        Cross,      // 十字方向に発射
        Targeted    // プレイヤーを狙って発射
    };

    /// <summary>
    /// 攻撃を実行する。
    /// </summary>
    void ExecuteAttack(AttackType type);
    /// <summary>
    /// 攻撃サイクルの更新。
    /// </summary>
    void UpdateAttackCycle(float dt);

private:
    std::vector<std::unique_ptr<CircusBullet>> bullets_;
    float fireTimer_ = 0.0f;
    float fireInterval_ = 5.0f;
    int missileCount_ = 24;
    int bulletMode_ = 0; // 0: Homing, 1: Around Player
    
    AttackType currentAttack_ = AttackType::Burst;
    bool autoCycle_ = true;
    float cycleTimer_ = 0.0f;
    const float kCycleInterval = 8.0f;

    // 弾の調整パラメータ
    float bulletSpeed_ = 0.6f;
    float bulletTurnSpeed_ = 0.08f;
    float bulletSwerveAmp_ = 0.4f;
    float bulletSwerveFreq_ = 5.0f;
    float bulletPhase1Duration_ = 0.4f;
    int bulletTargetDelayFrames_ = 25; // ターゲットの遅延フレーム数

    int maxHp_ = 100; // ボスの最大HP

    // スパイラル攻撃用
    float spiralAngle_ = 0.0f;

    // バースト連射用メンバ
    int remainingMissiles_ = 0;
    float burstTimer_ = 0.0f;
    const float kBurstDelay = 0.03f; // 1発ごとの間隔

    // ===== ボス本体の移動 =====
    void UpdateBossMovement(float dt);

    // 共通
    float moveTime_ = 0.0f;          // 移動用の累積時間
    TuboEngine::Math::Vector3 basePosition_; // 初回Update時に確定する基準座標
    bool basePositionInitialized_ = false;   // 基準位置が確定済みか

    // (2) ストレイフ（左右ゆっくりスライド）
    float strafeAmp_  = 3.5f;   // 振幅
    float strafeFreq_ = 0.2f;   // 周波数(Hz)

    // (3) オービット（プレイヤー周囲を回る）
    float orbitAngle_    = 0.0f;  // 現在の角度
    float orbitRadius_   = 12.0f; // プレイヤーからの距離
    float orbitSpeed_    = 0.3f;  // 角速度(rad/s)
    bool  isOrbiting_    = false; // オービット中か

    // (4) フェーズダッシュ（HP半分で一度だけ発動）
    bool  phaseDashTriggered_ = false; // 発動済みか
    bool  isDashing_          = false; // ダッシュ中か
    float dashTimer_          = 0.0f;
    const float kDashDuration = 0.35f; // ダッシュにかかる秒数
    TuboEngine::Math::Vector3 dashStart_;
    TuboEngine::Math::Vector3 dashTarget_;
    float dashShakeTimer_     = 0.0f;  // 到着後の小揺れ用

    std::mt19937 rng_{std::random_device{}()};
    bool disabledBaseBullet_ = false;
};
