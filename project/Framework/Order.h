#pragma once
#include"Framework.h"

/// <summary>
/// OrderクラスはFrameworkを継承し、
/// ゲームやアプリケーションの各フレームで呼ばれる主要な処理（初期化・更新・描画・終了）を実装します。
/// </summary>
class Order : public Framework
{
public:

	/// <summary>
	/// 初期化処理
	/// アプリケーション開始時に一度だけ呼ばれます。
	/// 必要なリソースの確保や初期状態の設定を行います。
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// 更新処理
	/// 毎フレーム呼ばれ、ゲームロジックや入力処理などを行います。
	/// </summary>
	void Update() override;

	/// <summary>
	/// 終了処理
	/// アプリケーション終了時に呼ばれ、リソースの解放などを行います。
	/// </summary>
	void Finalize() override;

	/// <summary>
	/// 描画処理
	/// 毎フレーム呼ばれ、画面への描画を行います。
	/// </summary>
	void Draw() override;

};
