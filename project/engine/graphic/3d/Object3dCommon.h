#pragma once
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
	void Initialize();

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
	
	void SetDefaultCamera(Camera* camera) { defaultCamera = camera; }
	Camera* GetDefaultCamera()const { return defaultCamera; }


	
private:
	
	
	
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

