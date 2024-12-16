#pragma once
#include"DirectXcommon.h"
class PSO;//前方宣言
class SpriteCommon
{
public:
	/*------------------------------------------------------------
			関数
	------------------------------------------------------------*/

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 共通描画設定
	/// </summary>
	void DrawSettingsCommon();
	/*---------------------------------------------------
			GETTER!
	---------------------------------------------------*/
	DirectXCommon* GetDxCommon()const { return dxCommon_; }

private:

	DirectXCommon* dxCommon_;
	PSO* pso = nullptr;


};

