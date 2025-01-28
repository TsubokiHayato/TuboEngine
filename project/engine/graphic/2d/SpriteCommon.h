#pragma once
#include"WinApp.h"
#include"DirectXcommon.h"
#include<memory>
class PSO;//前方宣言
class NoneBlendPSO;
class NormalBlendPSO;
class AddBlendPSO;
class MultiplyBlendPSO;
class SubtractBlendPSO;
class ScreenBlendPSO;
class SpriteCommon
{
public:
	/*------------------------------------------------------------
			関数
	------------------------------------------------------------*/

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(WinApp* winApp, DirectXCommon* dxCommon);

	/// <summary>
   /// 共通描画設定
   /// </summary>
   /// <param name="blendMode">ブレンドモード</param>
   /// <remarks>
   /// 0: NoneBlendPSO
   /// 1: NormalBlendPSO
   /// 2: AddBlendPSO
   /// 3: SubtractBlendPSO
   /// 4: MultiplyBlendPSO
   /// 5: ScreenBlendPSO
   /// </remarks>
	void DrawSettingsCommon(int blendMode);
	/*---------------------------------------------------
			GETTER!
	---------------------------------------------------*/
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	WinApp* GetWinApp()const { return winApp_; }

private:
	WinApp* winApp_ = nullptr;
	DirectXCommon* dxCommon_;
	PSO* pso = nullptr;
	NoneBlendPSO* noneBlendPSO = nullptr;//NoneBlendPSO
	NormalBlendPSO* normalBlendPSO = nullptr;//NormalBlendPSO
	AddBlendPSO* addBlendPSO = nullptr;//AddBlendPSO
	MultiplyBlendPSO* multiplyBlendPSO = nullptr;//MultiplyBlendPSO
	SubtractBlendPSO* subtractBlendPSO = nullptr;//SubtractBlendPSO
	ScreenBlendPSO* screenBlendPSO = nullptr;//ScreenBlendPSO

	int blenderMode_;//ブレンダーモード
};

