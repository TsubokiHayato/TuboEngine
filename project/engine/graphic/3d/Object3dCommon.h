#pragma once
#include"WinApp.h"
#include"DirectXCommon.h"
class PSO;
class NoneBlendPSO;
class NormalBlendPSO;
class AddBlendPSO;
class MultiplyBlendPSO;
class SubtractBlendPSO;
class ScreenBlendPSO;
class Camera;
class Object3dCommon
{

public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectX共通部分</param>
	void Initialize(WinApp* winApp,DirectXCommon* dxCommon);

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
			GETTER & SETTER
	---------------------------------------------------*/
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	WinApp* GetWinApp()const { return winApp_; }

	void SetDefaultCamera(Camera* camera) { defaultCamera = camera; }
	Camera* GetDefaultCamera()const { return defaultCamera; }


	
private:
	
	
	WinApp* winApp_ = nullptr;//ウィンドウズアプリケーション
	DirectXCommon* dxCommon_ = nullptr;//DirectX共通部分
	PSO* pso = nullptr;//PSO
	NoneBlendPSO* noneBlendPSO = nullptr;//NoneBlendPSO
	NormalBlendPSO* normalBlendPSO = nullptr;//NormalBlendPSO
	AddBlendPSO* addBlendPSO = nullptr;//AddBlendPSO
	MultiplyBlendPSO* multiplyBlendPSO = nullptr;//MultiplyBlendPSO
	SubtractBlendPSO* subtractBlendPSO = nullptr;//SubtractBlendPSO
	ScreenBlendPSO* screenBlendPSO = nullptr;//ScreenBlendPSO

	Camera* defaultCamera = nullptr;//デフォルトカメラ
	int blenderMode_;//ブレンダーモード

};

