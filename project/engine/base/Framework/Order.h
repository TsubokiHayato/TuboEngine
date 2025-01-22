#pragma once
#include"Framework.h"

class Order : public Framework
{
public:
	
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize()override;

	/// <summary>
	/// 更新
	/// </summary>
	void Update()override;

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize()override;

	/// <summary>
	/// 描画
	/// </summary>
	void Draw()override;

};

