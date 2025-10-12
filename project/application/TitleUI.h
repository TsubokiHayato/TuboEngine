#pragma once
#include"Sprite.h"
#include "TitleUIButton.h"
#include "SceneType.h"
#include <memory>
#include <vector>

/// <summary>
/// タイトル画面のUI全体を管理するクラス
/// </summary>
class TitleUI {
public:
	/// <summary>
	/// デストラクタ
	/// </summary>
	~TitleUI();

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize();

	/// <summary>
	/// 毎フレーム更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// セレクターアニメーション用タイマーの更新
	/// </summary>
	/// <param name="deltaTime">経過時間（秒）</param>
	void UpdateSelectorAnimTime(float deltaTime);

	/// <summary>
	/// 決定アニメーションの進行・終了判定
	/// </summary>
	/// <param name="deltaTime">経過時間（秒）</param>
	/// <returns>true:アニメーション中, false:アニメーションしていない</returns>
	bool HandleDecisionAnimation(float deltaTime);

	/// <summary>
	/// 入力処理（上下・決定）
	/// </summary>
	void HandleInput();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// ボタン位置の再計算・再配置
	/// </summary>
	void UpdateButtonPositions();

	/// <summary>
	/// 各ボタンのスプライト更新
	/// </summary>
	void UpdateButtonSprites();

	/// <summary>
	/// セレクタースプライトの位置・スケール更新
	/// </summary>
	void UpdateSelectorSprite();

	//===Getter&&Setter===//

	/// <summary>
	/// シーン遷移要求フラグ取得
	/// </summary>
	bool GetrRequestSceneChange_() const { return requestSceneChange_; }

	
	/// <summary>
	/// オプション表示状態取得
	/// </summary>
	bool GetShowOption() const { return showOption_; }

private:
	
	// タイトル画面のボタンリスト
	std::vector<std::unique_ptr<TitleUIButton>> buttons_;
	// 現在選択中のボタンインデックス
	int selectedIndex_ = 0;
	// 決定アニメーション用：決定時のインデックス
	int decisionIndex_ = -1;

	// 各ボタンのY座標リスト
	std::vector<float> buttonYPositions_;
	// セレクターのオフセット（ボタン横の表示位置調整用）
	std::vector<Vector2> selectorOffsets_;

	// 各種ボタン・セレクターのスプライト
	std::unique_ptr<Sprite> LogoSprite_;
	std::unique_ptr<Sprite> StartButtonSprite_;
	std::unique_ptr<Sprite> QuitButtonSprite_;

	// オプション表示中フラグ
	bool showOption_ = false;

	// ボタン間隔（Y方向）
	float buttonSpacing_ = 100.0f; // ボタン間隔（初期値）
	// 最初のボタンのY座標
	float buttonStartY_ = 400.0f; // 最初のボタンのY座標

	// UIの有効/無効フラグ
	bool isActive_ = true;
	// シーン遷移要求フラグ
	bool requestSceneChange_ = false;
	
	// セレクターアニメーション用タイマー
	float selectorAnimTime_ = 0.0f;
	// 決定アニメーション中フラグ
	bool selectorDecisionAnim_ = false;
	// 決定アニメーション用タイマー
	float selectorDecisionAnimTime_ = 0.0f;

	SceneType nextSceneType_ = SceneType::Title;


};