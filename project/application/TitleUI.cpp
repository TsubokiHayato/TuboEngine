#include "TitleUI.h"
#include"TextureManager.h"
#include "Input.h"
#include <cmath>

///-------------------------------------------///
/// デストラクタ
///-------------------------------------------///
TitleUI::~TitleUI() {
	// UIを非アクティブ化
	isActive_ = false;
	requestSceneChange_ = false;

	// ボタンリストをクリア
	buttons_.clear();
	// 各種スプライトのリセット
	LogoSprite_.reset();
	StartButtonSprite_.reset();
	QuitButtonSprite_.reset();

}

///-------------------------------------------///
/// 初期化処理
///-------------------------------------------///
void TitleUI::Initialize() {
	// 画面中央X座標（例: 1280x720想定）
	const float centerX = 1280.0f / 2.0f;

	//-----------------------------
	// ボタン生成・追加
	//-----------------------------
	// Startボタン（シーン遷移）
	buttons_.emplace_back(std::make_unique<TitleUIButton>("Start", [this]() {
		requestSceneChange_ = true;         // シーン遷移要求
		nextSceneType_ = SceneType::Select; // 遷移先シーン
	}));
	// Exitボタン（アプリ終了）
	buttons_.emplace_back(std::make_unique<TitleUIButton>("Exit", []() { PostQuitMessage(0); }));

	//-----------------------------
	// ボタンY座標の初期値設定
	//-----------------------------
	buttonYPositions_ = {300.0f, 400.0f}; // Start, Exit

	//-----------------------------
	// セレクターのオフセット設定
	//-----------------------------
	selectorOffsets_ = {
	    {-150.0f, 0.0f}, // Start横
	    {-150.0f, 0.0f}  // Exit横
	};


	///----------------------------
	///texture
	///----------------------------
	
	TextureManager::GetInstance()->LoadTexture("TitleUI/Start.png");
	TextureManager::GetInstance()->LoadTexture("TitleUI/Exit.png");
	//-----------------------------
	// 各種スプライトの生成・初期化
	//-----------------------------
	// Startボタン用スプライト
	StartButtonSprite_ = std::make_unique<Sprite>();
	StartButtonSprite_->Initialize("TitleUI/Start.png");
	StartButtonSprite_->SetAnchorPoint({0.5f, 0.5f}); // 中心揃え
	StartButtonSprite_->SetPosition({centerX, buttonYPositions_[0]});
	StartButtonSprite_->SetGetIsAdjustTextureSize(true);
	StartButtonSprite_->Update();

	// Exitボタン用スプライト
	QuitButtonSprite_ = std::make_unique<Sprite>();
	QuitButtonSprite_->Initialize("TitleUI/Exit.png");
	QuitButtonSprite_->SetAnchorPoint({0.5f, 0.5f});
	QuitButtonSprite_->SetPosition({centerX, buttonYPositions_[1]});
	QuitButtonSprite_->SetGetIsAdjustTextureSize(true);
	QuitButtonSprite_->Update();

	//-----------------------------
	// ロゴスプライト生成・初期化
	//-----------------------------
	LogoSprite_ = std::make_unique<Sprite>();
	LogoSprite_->Initialize("TitleUI/Title.png");
	LogoSprite_->SetAnchorPoint({0.5f, 0.0f});
	LogoSprite_->SetPosition({centerX, 100.0f});
	LogoSprite_->SetGetIsAdjustTextureSize(true);
	LogoSprite_->Update();

	//-----------------------------
	// 配列サイズの調整（安全のため）
	//-----------------------------
	buttonYPositions_.resize(buttons_.size());
	selectorOffsets_.resize(buttons_.size(), {-150.0f, 0.0f});

	//-----------------------------
	// 入力確認ボタン
	//-----------------------------
	UpdateButtonPositions();

}

///-------------------------------------------///
/// 毎フレーム更新処理
///-------------------------------------------///
void TitleUI::Update() {
	// UIが非アクティブなら何もしない
	if (!isActive_)
		return;

	//-----------------------------
	// アニメーション用タイマー更新
	//-----------------------------
	const float deltaTime = 1.0f / 60.0f; // 仮に60FPS固定
	UpdateSelectorAnimTime(deltaTime);

	//-----------------------------
	// 決定アニメーション処理
	//-----------------------------
	if (HandleDecisionAnimation(deltaTime)) {
		// アニメーション中は他の入力を受け付けない
		return;
	}

	//-----------------------------
	// 入力処理（上下・決定）
	//-----------------------------
	HandleInput();

	//-----------------------------
	// セレクタースプライトの更新
	//-----------------------------
	UpdateSelectorSprite();

	//-----------------------------
	// 各ボタンのスプライト更新
	//-----------------------------
	UpdateButtonSprites();

	LogoSprite_->Update();
	
}

///-------------------------------------------///
/// セレクターアニメーション用タイマーの更新
///-------------------------------------------///
void TitleUI::UpdateSelectorAnimTime(float deltaTime) {
	// セレクターのアニメーション用経過時間を加算
	selectorAnimTime_ += deltaTime;
}

///-------------------------------------------///
/// 決定アニメーションの進行・終了判定
///-------------------------------------------///
bool TitleUI::HandleDecisionAnimation(float deltaTime) {
	if (selectorDecisionAnim_) {
		// 決定アニメーション中
		selectorDecisionAnimTime_ += deltaTime;
		if (selectorDecisionAnimTime_ > 0.3f) { // 0.3秒でアニメーション終了
			selectorDecisionAnim_ = false;
			selectorDecisionAnimTime_ = 0.0f;
			// アニメーション終了時にClickを呼ぶ
			if (decisionIndex_ >= 0 && decisionIndex_ < static_cast<int>(buttons_.size())) {
				buttons_[decisionIndex_]->Click();
			}
			decisionIndex_ = -1;
		}
		return true; // アニメーション中
	}
	return false; // アニメションしていない
}

///-------------------------------------------///
/// 入力処理（上下・決定）
///-------------------------------------------///
void TitleUI::HandleInput() {
    // 上キー：選択インデックスを1つ上へ
    if (Input::GetInstance()->TriggerKey(DIK_UP) || Input::GetInstance()->TriggerKey(DIK_W)) {
        selectedIndex_ = (selectedIndex_ + static_cast<int>(buttons_.size()) - 1) % static_cast<int>(buttons_.size());
    }
    // 下キー：選択インデックスを1つ下へ
	if (Input::GetInstance()->TriggerKey(DIK_DOWN) || Input::GetInstance()->TriggerKey(DIK_S)) {
		selectedIndex_ = (selectedIndex_ + 1) % static_cast<int>(buttons_.size());
	}
	// 決定ボタン：決定アニメーション開始
	if (Input::GetInstance()->TriggerKey(DIK_RETURN) || Input::GetInstance()->TriggerKey(DIK_SPACE)) {
        selectorDecisionAnim_ = true;
        selectorDecisionAnimTime_ = 0.0f;
        decisionIndex_ = selectedIndex_;
    }
}


///-------------------------------------------///
/// セレクタースプライトの位置・スケール更新
///-------------------------------------------///
void TitleUI::UpdateSelectorSprite() {
	const float centerX = 1280.0f / 2.0f;
	// --- スケールアニメーション ---
	float scale = 1.0f;
	if (selectorDecisionAnim_) {
		// 決定時は一瞬大きくしてから戻す
		float t = selectorDecisionAnimTime_ / 0.3f;
		scale = 1.0f + 0.5f * std::sin(t * 3.14159f); // 0→1→0
	} else {
		// 通常はゆっくり拡大縮小
		scale = 1.0f + 0.1f * std::sin(selectorAnimTime_ * 2.0f * 3.14159f); // 1±0.1
	}
}

///-------------------------------------------///
/// 各ボタンのスプライト更新
///-------------------------------------------///
void TitleUI::UpdateButtonSprites() {
    // 色定義
    const Vector4 selectedColor = {1.0f, 0.8f, 0.2f, 1.0f}; // 選択中（黄色系）
    const Vector4 normalColor   = {1.0f, 1.0f, 1.0f, 1.0f}; // 通常（白）

    // Startボタン
    if (StartButtonSprite_) {
        if (selectedIndex_ == 0) {
            StartButtonSprite_->SetColor(selectedColor);
        } else {
            StartButtonSprite_->SetColor(normalColor);
        }
        StartButtonSprite_->Update();
    }
    // Quitボタン
    if (QuitButtonSprite_) {
        if (selectedIndex_ == 1) {
            QuitButtonSprite_->SetColor(selectedColor);
        } else {
            QuitButtonSprite_->SetColor(normalColor);
        }
        QuitButtonSprite_->Update();
    }
}

///-------------------------------------------///
/// ボタン位置の再計算・再配置
///-------------------------------------------///
void TitleUI::UpdateButtonPositions() {
	const float centerX = 1280.0f / 2.0f;
	// 必ずbuttonYPositions_のサイズをbuttons_に合わせる
	buttonYPositions_.resize(buttons_.size());
	for (size_t i = 0; i < buttons_.size(); ++i) {
		// Y座標を計算
		float y = buttonStartY_ + buttonSpacing_ * static_cast<float>(i);
		// 各ボタンのスプライト位置を設定
		switch (i) {
		case 0:
			StartButtonSprite_->SetPosition({centerX, y});
			break;
		case 1:
			QuitButtonSprite_->SetPosition({centerX, y});
			break;
		}
		buttonYPositions_[i] = y;
	}
}

///-------------------------------------------///
/// 描画処理
///-------------------------------------------///
void TitleUI::Draw() {
	if (!isActive_)
		return;


	// ロゴ
	if (LogoSprite_) {
		LogoSprite_->Draw();
	}

	// 各ボタンのスプライト描画
	StartButtonSprite_->Draw();
	QuitButtonSprite_->Draw();
}
