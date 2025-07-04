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
	/// シングルトンインスタンス取得
	/// </summary>
	static Object3dCommon* GetInstance() {
		static Object3dCommon instance;
		return &instance;
	}

private:
	// コンストラクタ・デストラクタ・コピー禁止
	Object3dCommon() = default;
	~Object3dCommon() = default;
	Object3dCommon(const Object3dCommon&) = delete;
	Object3dCommon& operator=(const Object3dCommon&) = delete;

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

