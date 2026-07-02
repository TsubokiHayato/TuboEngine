#pragma once
#include "Object3d.h"
#include "Vector3.h"
#include"engine/Collider/Collider.h"


/// <summary>
/// キャラクター（プレイヤー・敵）の基底クラス。位置・回転などの共通状態と当たり判定（Collider）を提供する。
/// </summary>
class BaseCharacter : public Collider {

public:
	///--------------------------------------------------
	///				メンバ関数
	///--------------------------------------------------

	/// <summary>
	/// コンストラクタ。
	/// </summary>
	BaseCharacter();
	/// <summary>
	/// デストラクタ。
	/// </summary>
	virtual ~BaseCharacter();


	/// <summary>
	/// 初期化
	/// </summary>
	virtual void Initialize(
	);

	/// <summary>
	/// 終了処理
	/// </summary>
	virtual void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	virtual void Draw();
	
	/// <summary>
	///　衝突判定
	/// </summary>
	/// <param name="other"></param>
	virtual void OnCollision(Collider* other) override;

	/// <summary>
	/// 当たり判定の中心座標を取得
	virtual TuboEngine::Math::Vector3 GetCenterPosition() const override;
	


	///--------------------------------------------------
	///				メンバ変数
	/// -------------------------------------------------
	
	

};
