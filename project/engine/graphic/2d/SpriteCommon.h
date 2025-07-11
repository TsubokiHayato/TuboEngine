#pragma once
#include<memory>
#include"PSO.h"
#include"NormalBlendPSO.h"
#include"AddBlendPSO.h"
#include"SubtractBlendPSO.h"
#include"MultiplyBlendPSO.h"
#include"ScreenBlendPSO.h"

class SpriteCommon
{
public:
	/// <summary>
	/// シングルトンインスタンス取得
	/// </summary>
	static SpriteCommon* GetInstance() {
		if (!instance) {
			instance = new SpriteCommon();
		}
		return instance;
	}

private:
	// コンストラクタ・デストラクタ・コピー禁止
	static SpriteCommon* instance; // シングルトンインスタンス
	SpriteCommon() = default;
	~SpriteCommon() = default;
	SpriteCommon(const SpriteCommon&) = delete;
	SpriteCommon& operator=(const SpriteCommon&) = delete;

public:
	/*------------------------------------------------------------
			関数
	------------------------------------------------------------*/

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
	
private:

	std::unique_ptr <PSO> pso = nullptr;//PSOのユニークポインタ
	std::unique_ptr <NormalBlendPSO > normalBlendPSO;//NormalBlendPSOのユニークポインタ
	std::unique_ptr <AddBlendPSO > addBlendPSO;//AddBlendPSOのユニークポインタ
	std::unique_ptr <MultiplyBlendPSO > multiplyBlendPSO;//MultiplyBlendPSOのユニークポインタ
	std::unique_ptr <SubtractBlendPSO > subtractBlendPSO;//SubtractBlendPSOのユニークポインタ
	std::unique_ptr <ScreenBlendPSO > screenBlendPSO;//ScreenBlendPSOのユニークポインタ


	int blenderMode_;//ブレンダーモード
};

