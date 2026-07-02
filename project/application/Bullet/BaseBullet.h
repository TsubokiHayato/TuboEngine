#pragma once
#include "Object3d.h"
#include "Object3dCommon.h"
#include "Vector3.h"
#include "engine/Collider/Collider.h"

/// <summary>
/// 弾（ブレット）の基底クラス。
/// 位置・速度・生存フラグなどの共通状態と、当たり判定（Collider）を提供し、
/// 個々の弾種は Initialize/Update/Draw を派生先で実装する。
/// </summary>
class BaseBullet : public Collider {
public:
    /// <summary>
    /// デストラクタ。
    /// </summary>
    virtual ~BaseBullet() = default;

    /// <summary>
    /// 初期化処理。開始位置を設定し、弾を発生させる。
    /// </summary>
    /// <param name="startPos">弾の初期位置</param>
    virtual void Initialize(const TuboEngine::Math::Vector3& startPos) = 0;

    /// <summary>
    /// 更新処理。
    /// </summary>
    virtual void Update() = 0;

    /// <summary>
    /// 描画処理。
    /// </summary>
    virtual void Draw()   = 0;

    /// <summary>
    /// 衝突時の処理。
    /// </summary>
    virtual void OnCollision(Collider* other) override;
    /// <summary>
    /// 中心座標を取得する。
    /// </summary>
    virtual TuboEngine::Math::Vector3 GetCenterPosition() const override;

    // ---- 共通アクセサ ----
    /// <summary>生存フラグの取得・設定。</summary>
    bool GetIsAlive() const { return isAlive; }
    void SetIsAlive(bool alive) { isAlive = alive; }

    /// <summary>座標の取得・設定。</summary>
    const TuboEngine::Math::Vector3& GetPosition() const { return position; }
    void SetPosition(const TuboEngine::Math::Vector3& p) { position = p; }

    /// <summary>回転の取得・設定。</summary>
    const TuboEngine::Math::Vector3& GetRotation() const { return rotation; }
    void SetRotation(const TuboEngine::Math::Vector3& r) { rotation = r; }

    /// <summary>スケールの取得・設定。</summary>
    const TuboEngine::Math::Vector3& GetScale() const { return scale; }
    void SetScale(const TuboEngine::Math::Vector3& s) { scale = s; }

    /// <summary>速度の取得・設定。</summary>
    const TuboEngine::Math::Vector3& GetVelocity() const { return velocity; }
    void SetVelocity(const TuboEngine::Math::Vector3& v) { velocity = v; }

    /// <summary>描画に使用するカメラを設定する。</summary>
    void SetCamera(TuboEngine::Camera* camera) { if (object3d) object3d->SetCamera(camera); }

protected:
    TuboEngine::Math::Vector3 position;
    TuboEngine::Math::Vector3 velocity;
    TuboEngine::Math::Vector3 rotation;
    TuboEngine::Math::Vector3 scale;

    bool  isAlive     = false;
    bool  isHit       = false;
    float bulletSpeed = 0.0f;
    float disappearZ  = 100.0f;

    int reflectCount    = 0;
    int maxReflectCount = 2;

    std::unique_ptr<TuboEngine::Object3d> object3d;

    // position/rotation/scale を object3d に反映して Update() する共通処理
    void ApplyTransformToObject3d();
};
