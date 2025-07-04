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
	void Initialize();

	/// <summary>
    /// 共通描画設定
    /// </summary>
	void DrawSettingsCommon();
	
private:
	SkyBoxPSO* pso = nullptr;
};

