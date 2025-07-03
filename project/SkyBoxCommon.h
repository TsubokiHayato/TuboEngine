#pragma once


#pragma once
#include"WinApp.h"
#include"DirectXcommon.h"
#include<memory>
class SkyBoxPSO;//前方宣言


class SkyBoxCommon
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
	void DrawSettingsCommon();
	/*---------------------------------------------------
			GETTER!
	---------------------------------------------------*/
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	WinApp* GetWinApp()const { return winApp_; }

private:
	WinApp* winApp_ = nullptr;
	DirectXCommon* dxCommon_;
	SkyBoxPSO* pso = nullptr;
};

