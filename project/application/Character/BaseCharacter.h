#pragma once
#include "Object3d.h"
#include "Vector3.h"
#include"Collider/Collider.h"


class BaseCharacter : public Collider {

public:
	///--------------------------------------------------
	///				メンバ関数
	///--------------------------------------------------

	BaseCharacter();
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
	virtual Vector3 GetCenterPosition() const override;
	


	///--------------------------------------------------
	///				メンバ変数
	/// -------------------------------------------------
	
	

};
