#pragma once
#include "BaseBullet.h"
#include "Object3d.h"
#include "Vector3.h"

class Player;

///--------------------------------------------------
/// プレイヤーの弾クラス
///--------------------------------------------------
class PlayerBullet : public BaseBullet {

public:
	///--------------------------------------------------
	///				メンバ関数
	///--------------------------------------------------

	// 初期化処理
	void Initialize(const Vector3& startPos) override;

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	// ImGuiによるグローバル描画
	static void DrawImGuiGlobal();

	/// <summary>
	/// 　衝突判定
	/// </summary>
	/// <param name="other"></param>
	// 衝突時の処理
	virtual void OnCollision(Collider* other) override;

	/// <summary>
	/// 当たり判定の中心座標を取得
	/// </summary>
	// 当たり判定の中心座標を返す
	virtual Vector3 GetCenterPosition() const override;

	///--------------------------------------------------
	///				ゲッター&セッター
	///--------------------------------------------------

	// 生存判定
	bool GetIsAlive() const { return isAlive; }

	// 位置取得
	const Vector3& GetPosition() const { return position; }
	// 位置設定
	void SetPosition(const Vector3& position) { this->position = position; }

	// 回転取得
	const Vector3& GetRotation() const { return rotation; }
	// 回転設定
	void SetRotation(const Vector3& rotation) { this->rotation = rotation; }

	// スケール取得
	const Vector3& GetScale() const { return scale; }
	// スケール設定
	void SetScale(const Vector3& scale) { this->scale = scale; }

	// 速度取得
	const Vector3& GetVelocity() const { return velocity; }
	// 速度設定
	void SetVelocity(const Vector3& velocity) { this->velocity = velocity; }

	// カメラ設定
	void SetCamera(Camera* camera) { object3d->SetCamera(camera); }

	// プレイヤーの位置を設定
	void SetPlayerPosition(const Vector3& playerPosition) { playerPostion_ = playerPosition; }
	// プレイヤーの回転を設定
	void SetPlayerRotation(const Vector3& rotation) { playerRotation = rotation; }

private:
	///--------------------------------------------------
	///				メンバ変数
	///--------------------------------------------------

	
	///  弾のパラメータ ///
	// 弾の位置
	Vector3 position;
	// 弾の回転
	Vector3 rotation;
	// 弾のスケール
	Vector3 scale;
	// 弾の速度
	Vector3 velocity;

private:
	// 弾の速度
	float bulletSpeed = 2.0f;
	// 消滅するZ座標
	float disappearZ = 100.0f;

	// 生存フラグ
	bool isAlive = true;
	// ヒット判定フラグ
	bool isHit = false;

	/// Player関連 ///
	// プレイヤーの位置
	Vector3 playerPostion_ = {};
	// プレイヤーの回転
	Vector3 playerRotation = {};

public:
	///--------------------------------------------------
	///				静的メンバ変数
	///--------------------------------------------------

	// プレイヤーからの消滅半径
	static float s_disappearRadius;
	// 弾の速度
	static float s_bulletSpeed;
	// 消滅するZ座標
	static float s_disappearZ;
	// 弾のスケール
	static Vector3 s_scale;
	// 弾の回転
	static Vector3 s_rotation;

	// ダメージ量
	static int s_damage;
	// 発射間隔（秒）
	static float s_fireInterval;
};
