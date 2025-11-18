#pragma once
#include "Character/Enemy/Enemy.h"
#include "Vector3.h"
#include <memory>

class Player;
class Object3d;
class Camera;

class ChargingEnemy : public Enemy {
public:
	ChargingEnemy();
	~ChargingEnemy() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void DrawImGui();

	// セッター（基底と重複するものは基底を使ってください）
	// SetPlayer / SetCamera / SetPosition は基底 Enemy のメソッドを使う

	void SetCamera(Camera* camera) { camera_ = camera; }
	// 衝突処理
	void OnCollision(Collider* other) override;
	Vector3 GetCenterPosition() const override;

private:

	Camera* camera_ = nullptr;
	enum class State {
		Idle,
		Detect,
		Windup,  // 準備（テレグラフ）
		Charge,  // 突進中
		Recover, // クールダウン
	};

	// 突進パラメータ（チューニング可能）
	float detectRadius_ = 6.0f;   // プレイヤー発見距離
	float windupTime_ = 0.6f;     // テレグラフ時間
	float chargeSpeed_ = 0.6f;    // 突進速度（1フレームあたり、調整可）
	float chargeDuration_ = 0.5f; // 突進継続時間（秒）
	float recoverTime_ = 0.8f;    // 回復時間（秒）

	// 実行用変数
	float stateTimer_ = 0.0f;
	Vector3 chargeDir_{0.0f, 0.0f, 0.0f};
	bool isAlive_ = true;
	bool isHit_ = false;

	// ChargingEnemy 用に独自オブジェクトを持つ（基底 Enemy と重複しない名前）
	std::unique_ptr<Object3d> object3d_;
	State state_ = State::Idle;
};