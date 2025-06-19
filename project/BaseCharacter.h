#pragma once
#include "Object3d.h"
#include "Vector3.h"

class BaseCharacter {

public:
	///--------------------------------------------------
	///				メンバ関数
	///--------------------------------------------------

	BaseCharacter();
	virtual ~BaseCharacter();


	/// <summary>
	/// 初期化
	/// </summary>
	virtual void Initialize(Object3dCommon* object3dCommon);

	/// <summary>
	/// 終了処理
	/// </summary>
	virtual void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	virtual void Draw();
	
	/// <summary>
	/// 終了処理
	/// </summary>
	virtual void Finalize();

	
	/// <summary>
	/// 移動処理
	/// </summary>
	virtual void Move();

	/// <summary>
	/// ダメージ処理
	/// </summary>
	/// <param name="damage"></param>
	virtual void TakeDamage(int damage);

	
	///--------------------------------------------------
	///				メンバ変数
	/// -------------------------------------------------
	
	

};
