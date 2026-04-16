#pragma once
#include "Bullet/BaseBullet.h"
#include "Object3d.h"
#include "Vector3.h"
#include"MapChip/MapChipField.h"

///---------------------------------------------------
//				前方宣言
///---------------------------------------------------
class Enemy;
class Player;

///--------------------------------------------------
/// プレイヤーの弾クラス
///--------------------------------------------------
class EnemyNormalBullet : public BaseBullet {

public:
	///--------------------------------------------------
	///				メンバ関数
	///--------------------------------------------------

	// 初期化処理
	void Initialize(const TuboEngine::Math::Vector3& startPos) override;

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
	virtual TuboEngine::Math::Vector3 GetCenterPosition() const override;

	// 追加: マップチップフィールド設定
	void SetMapChipField(MapChipField* field) { mapChipField_ = field; }

	///--------------------------------------------------
	///				ゲッター&セッター
	///--------------------------------------------------

	// 生存判定
	bool GetIsAlive() const { return isAlive; }

	// 位置取得
	const TuboEngine::Math::Vector3& GetPosition() const { return position; }
	// 位置設定
	void SetPosition(const TuboEngine::Math::Vector3& position) { this->position = position; }

	// 回転取得
	const TuboEngine::Math::Vector3& GetRotation() const { return rotation; }
	// 回転設定
	void SetRotation(const TuboEngine::Math::Vector3& rotation) { this->rotation = rotation; }

	// スケール取得
	const TuboEngine::Math::Vector3& GetScale() const { return scale; }
	// スケール設定
	void SetScale(const TuboEngine::Math::Vector3& scale) { this->scale = scale; }

	// 速度取得
	const TuboEngine::Math::Vector3& GetVelocity() const { return velocity; }
	// 速度設定
	void SetVelocity(const TuboEngine::Math::Vector3& velocity) { this->velocity = velocity; }

	// カメラ設定
	void SetCamera(TuboEngine::Camera* camera) { object3d->SetCamera(camera); }

	// プレイヤーのポインタを設定
	void SetPlayer(Player* player) { player_ = player; }
	
	// 敵の位置を設定
	void SetEnemyPosition(const TuboEngine::Math::Vector3& position) { enemyPosition_ = position; }
	// 敵の回転を設定
	void SetEnemyRotation(const TuboEngine::Math::Vector3& rotation) { enemyRotation_ = rotation; }
	//敵の生存状態を設定
	void SetIsAlive(bool isAlive) { this->isAlive = isAlive; }


private:
	///--------------------------------------------------
	///				メンバ変数
	///--------------------------------------------------

	///  弾のパラメータ ///
	// 弾の位置
	TuboEngine::Math::Vector3 position;
	// 弾の回転
	TuboEngine::Math::Vector3 rotation;
	// 弾のスケール
	TuboEngine::Math::Vector3 scale;
	// 弾の速度
	TuboEngine::Math::Vector3 velocity;

private:
	// 弾の速度
	float bulletSpeed = 0.0f;
	// 消滅するZ座標
	float disappearZ = 0.0f;

	// 生存フラグ
	bool isAlive = true;
	// ヒット判定フラグ
	bool isHit = false;

	/// Player ///
	Player* player_ = nullptr; // プレイヤーへのポインタ

	/// Enemy ///
	TuboEngine::Math::Vector3 enemyPosition_; // 敵の位置
	TuboEngine::Math::Vector3 enemyRotation_; // 敵の回転

	// 参照用
	MapChipField* mapChipField_ = nullptr;

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
	static TuboEngine::Math::Vector3 s_scale;
	// 弾の回転
	static TuboEngine::Math::Vector3 s_rotation;

	// ダメージ量
	static int s_damage;
	// 発射間隔（秒）
	static float s_fireInterval;
};
