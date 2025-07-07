#pragma once
#include"PSO.h"
#include"NoneBlendPSO.h"
#include"NormalBlendPSO.h"
#include"AddBlendPSO.h"
#include"SubtractBlendPSO.h"
#include"MultiplyBlendPSO.h"
#include"ScreenBlendPSO.h"


class Camera;
class Object3dCommon
{
public:
	/// <summary>
	/// シングルトンインスタンス取得
	/// </summary>
	static Object3dCommon* GetInstance() {

		if (instance == nullptr) {
			instance = new Object3dCommon();
		}
		return instance;
	}

private:
	// コンストラクタ・デストラクタ・コピー禁止
	static Object3dCommon* instance; // シングルトンインスタンス
	Object3dCommon() = default;
	~Object3dCommon() = default;
	Object3dCommon(const Object3dCommon&) = delete;
	Object3dCommon& operator=(const Object3dCommon&) = delete;

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	void Finalize();

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


	std::unique_ptr <PSO> pso = nullptr;//PSOのユニークポインタ
	std::unique_ptr <NoneBlendPSO > noneBlendPSO;//NoneBlendPSOのユニークポインタ
	std::unique_ptr <NormalBlendPSO > normalBlendPSO;//NormalBlendPSOのユニークポインタ
	std::unique_ptr <AddBlendPSO > addBlendPSO;//AddBlendPSOのユニークポインタ
	std::unique_ptr <MultiplyBlendPSO > multiplyBlendPSO;//MultiplyBlendPSOのユニークポインタ
	std::unique_ptr <SubtractBlendPSO > subtractBlendPSO;//SubtractBlendPSOのユニークポインタ
	std::unique_ptr <ScreenBlendPSO > screenBlendPSO;//ScreenBlendPSOのユニークポインタ




	Camera* defaultCamera = nullptr;//デフォルトカメラ
	int blenderMode_;//ブレンダーモード

};

