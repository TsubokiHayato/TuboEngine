#include "GuideUI.h"

#include "Input.h"
#include "TextureManager.h"

namespace {
	// 16pxフォント想定のコンパクトUI
	constexpr float kKeySize = 32.0f;
	constexpr float kKeyGap = 4.0f;
	constexpr float kMargin = 20.0f;
	constexpr float kLabelH = 16.0f;
	constexpr float kLabelW = 96.0f;
 	constexpr float kSpaceW = 48.0f; // fixed layout width (actual sprite will use texture size)
 	constexpr float kSpaceH = 40.0f; // 行の高さを少し低めに
	constexpr float kMouseDead = 2.0f;
	constexpr float kLabelGapX = 6.0f;
	constexpr float kRowGap = 4.0f;
	constexpr float kMouseW = 40.0f;
	constexpr float kMouseH = 40.0f;
	constexpr float kMouseShootW = 40.0f;
	constexpr float kMouseShootH = 40.0f;
	inline float ClampF(float v, float mn, float mx) { return (v < mn) ? mn : (v > mx) ? mx : v; }
}

static float AbsF(float v) { return (v >= 0.0f) ? v : -v; }

const std::string& GuideUI::SelectMouseIconTexture(const Input::MouseMove& move) const {
	const float ax = AbsF(static_cast<float>(move.lX));
	const float ay = AbsF(static_cast<float>(move.lY));

	// small vs move のみ
	if (ax < kMouseDead && ay < kMouseDead) {
		return texMouseSmall_;
	}
	return texMouseMove_;
}

void GuideUI::Initialize() {
	// texture paths
	texW_ = "external/Keyboard & Mouse/Default/keyboard_w.png";
	texWOutline_ = "external/Keyboard & Mouse/Default/keyboard_w_outline.png";
	texA_ = "external/Keyboard & Mouse/Default/keyboard_a.png";
	texAOutline_ = "external/Keyboard & Mouse/Default/keyboard_a_outline.png";
	texS_ = "external/Keyboard & Mouse/Default/keyboard_s.png";
	texSOutline_ = "external/Keyboard & Mouse/Default/keyboard_s_outline.png";
	texD_ = "external/Keyboard & Mouse/Default/keyboard_d.png";
	texDOutline_ = "external/Keyboard & Mouse/Default/keyboard_d_outline.png";
	texSpace_ = "external/Keyboard & Mouse/Default/keyboard_space.png";
	texSpaceOutline_ = "external/Keyboard & Mouse/Default/keyboard_space_outline.png";
	texMouseSmall_ = "external/Keyboard & Mouse/Default/mouse_small.png";
	texMouseMove_ = "external/Keyboard & Mouse/Default/mouse_move.png";
	texMouse_ = "external/Keyboard & Mouse/Default/mouse_left.png";
	texMouseRight_ = "external/Keyboard & Mouse/Default/mouse_right.png";

	TextureManager::GetInstance()->LoadTexture(texW_);
	TextureManager::GetInstance()->LoadTexture(texWOutline_);
	TextureManager::GetInstance()->LoadTexture(texA_);
	TextureManager::GetInstance()->LoadTexture(texAOutline_);
	TextureManager::GetInstance()->LoadTexture(texS_);
	TextureManager::GetInstance()->LoadTexture(texSOutline_);
	TextureManager::GetInstance()->LoadTexture(texD_);
	TextureManager::GetInstance()->LoadTexture(texDOutline_);
	TextureManager::GetInstance()->LoadTexture(texSpace_);
	TextureManager::GetInstance()->LoadTexture(texSpaceOutline_);
	TextureManager::GetInstance()->LoadTexture(texMouseSmall_);
	TextureManager::GetInstance()->LoadTexture(texMouseMove_);
	TextureManager::GetInstance()->LoadTexture(texMouse_);
	TextureManager::GetInstance()->LoadTexture(texMouseRight_);

	// Guide label textures
	const std::string texMoveGuide = "GuideUI/moveGuide.png";
	const std::string texDashGuide = "GuideUI/DashGuide.png";
	const std::string texAimGuide = "GuideUI/AimGuide.png";
	const std::string texShotGuide = "GuideUI/shotGuide.png";
	TextureManager::GetInstance()->LoadTexture(texMoveGuide);
	TextureManager::GetInstance()->LoadTexture(texDashGuide);
	TextureManager::GetInstance()->LoadTexture(texAimGuide);
	TextureManager::GetInstance()->LoadTexture(texShotGuide);

	keyW_ = std::make_unique<Sprite>();
	keyW_->Initialize(texW_);
	keyW_->SetAnchorPoint({ 0.0f, 0.0f });
	keyW_->SetSize({ kKeySize, kKeySize });

	keyA_ = std::make_unique<Sprite>();
	keyA_->Initialize(texA_);
	keyA_->SetAnchorPoint({ 0.0f, 0.0f });
	keyA_->SetSize({ kKeySize, kKeySize });

	keyS_ = std::make_unique<Sprite>();
	keyS_->Initialize(texS_);
	keyS_->SetAnchorPoint({ 0.0f, 0.0f });
	keyS_->SetSize({ kKeySize, kKeySize });

	keyD_ = std::make_unique<Sprite>();
	keyD_->Initialize(texD_);
	keyD_->SetAnchorPoint({ 0.0f, 0.0f });
	keyD_->SetSize({ kKeySize, kKeySize });

	labelMove_ = std::make_unique<Sprite>();
	labelMove_->Initialize(texMoveGuide);
	labelMove_->SetAnchorPoint({ 0.0f, 0.0f });
	labelMove_->SetGetIsAdjustTextureSize(true);
	labelMove_->Update();

	// SPACE (Dash)
	keySpace_ = std::make_unique<Sprite>();
	keySpace_->Initialize(texSpace_);
	keySpace_->SetAnchorPoint({ 0.0f, 0.0f });
	keySpace_->SetGetIsAdjustTextureSize(true); // テクスチャの実サイズにする（引き伸ばさない）
	keySpace_->Update();

	labelDash_ = std::make_unique<Sprite>();
	labelDash_->Initialize(texDashGuide);
	labelDash_->SetAnchorPoint({ 0.0f, 0.0f });
	labelDash_->SetGetIsAdjustTextureSize(true);
	labelDash_->Update();

	// Mouse (Aim)
	mouseIcon_ = std::make_unique<Sprite>();
	mouseIcon_->Initialize(texMouseSmall_);
	mouseIcon_->SetAnchorPoint({ 0.0f, 0.0f });
	mouseIcon_->SetSize({ kMouseW, kMouseH });

	labelAim_ = std::make_unique<Sprite>();
	labelAim_->Initialize(texAimGuide);
	labelAim_->SetAnchorPoint({ 0.0f, 0.0f });
	labelAim_->SetGetIsAdjustTextureSize(true);
	labelAim_->Update();

	// Mouse (Shoot)
	mouseShootIcon_ = std::make_unique<Sprite>();
	mouseShootIcon_->Initialize(texMouse_);
	mouseShootIcon_->SetAnchorPoint({ 0.0f, 0.0f });
	mouseShootIcon_->SetSize({ kMouseShootW, kMouseShootH });

	labelShoot_ = std::make_unique<Sprite>();
	labelShoot_->Initialize(texShotGuide);
	labelShoot_->SetAnchorPoint({ 0.0f, 0.0f });
	labelShoot_->SetGetIsAdjustTextureSize(true);
	labelShoot_->Update();

	// 左下: 画面左下からのマージンで配置
	basePos_ = { kMargin, kMargin + 200.0f };

	Update();
}

void GuideUI::UpdateKeySpriteState(Sprite* sprite, const std::string& normalTex, const std::string& outlineTex, bool pressed) const {
	if (!sprite) {
		return;
	}
	sprite->SetTexture(pressed ? outlineTex : normalTex);
}

void GuideUI::Update() {
	if (!isActive_) {
		return;
	}

	const float screenW = static_cast<float>(WinApp::GetInstance()->GetClientWidth());
	const float screenH = static_cast<float>(WinApp::GetInstance()->GetClientHeight());
	const float cell = (kKeySize + kKeyGap);
	const float rowGap = kRowGap;
	const float labelGapX = kLabelGapX;

	// clamp sizes
	const float topBlockWidth = 3.0f * cell + 10.0f + kLabelW;
	const float bottomBlockWidth = kSpaceW + labelGapX + kLabelW;
	const float mouseBlockWidth = kMouseW + labelGapX + kLabelW;
	const float mouseShootBlockWidth = kMouseShootW + labelGapX + kLabelW;
	float uiWidth = topBlockWidth;
	if (uiWidth < bottomBlockWidth) uiWidth = bottomBlockWidth;
	if (uiWidth < mouseBlockWidth) uiWidth = mouseBlockWidth;
	if (uiWidth < mouseShootBlockWidth) uiWidth = mouseShootBlockWidth;
	const float uiHeight = (cell + kKeySize) + rowGap + kSpaceH + rowGap + kMouseH + rowGap + kMouseShootH;

	// clamp
	const float minBaseX = 0.0f;
	const float minBaseY = 0.0f;
	const float maxBaseX = (screenW > uiWidth) ? (screenW - uiWidth) : 0.0f;
	const float maxBaseY = (screenH > uiHeight) ? (screenH - uiHeight) : 0.0f;
	const float clampedBaseX = ClampF(basePos_.x, minBaseX, maxBaseX);
	const float clampedBaseY = ClampF(basePos_.y, minBaseY, maxBaseY);

	// 配置基準（左上原点）
	const float left = clampedBaseX;
	const float baseTopY = screenH - clampedBaseY;

	// WASD block
	const float rowBottomTopY = baseTopY;
	const float rowTopTopY = baseTopY - cell;

	// W
	if (keyW_) {
		keyW_->SetPosition({ left + cell, rowTopTopY });
	}
	// A S D
	if (keyA_) {
		keyA_->SetPosition({ left + 0.0f, rowBottomTopY });
	}
	if (keyS_) {
		keyS_->SetPosition({ left + cell, rowBottomTopY });
	}
	if (keyD_) {
		keyD_->SetPosition({ left + 2.0f * cell, rowBottomTopY });
	}
	if (labelMove_) {
		labelMove_->SetPosition({ left + 3.0f * cell + labelGapX, rowBottomTopY });
	}

	// SPACE block
	const float spaceTopY = rowBottomTopY + rowGap + kSpaceH;
	if (keySpace_) {
		keySpace_->SetPosition({ left + 0.0f, spaceTopY });
	}
	if (labelDash_) {
		labelDash_->SetPosition({ left + kSpaceW + labelGapX, spaceTopY });
	}

	// MOUSE block
	const float mouseTopY = spaceTopY + rowGap + kMouseH;
	if (mouseIcon_) {
		mouseIcon_->SetPosition({ left + 0.0f, mouseTopY });
	}
	if (labelAim_) {
		labelAim_->SetPosition({ left + kMouseW + labelGapX, mouseTopY });
	}

	// MOUSE SHOOT block
	const float mouseShootTopY = mouseTopY + rowGap + kMouseShootH;
	if (mouseShootIcon_) {
		mouseShootIcon_->SetPosition({ left + 0.0f, mouseShootTopY });
	}
	if (labelShoot_) {
		labelShoot_->SetPosition({ left + kMouseShootW + labelGapX, mouseShootTopY });
	}

	auto* input = Input::GetInstance();

	// マウスアイコンは移動方向で切り替える
	if (mouseIcon_) {
		const auto move = input->GetMouseMove();
		mouseIcon_->SetTexture(SelectMouseIconTexture(move));
	}
	// 射撃: 右クリック押下ならmouse_right、それ以外はmouse（左用）
	if (mouseShootIcon_) {
		const bool right = input->IsPressMouse(1);
		mouseShootIcon_->SetTexture(right ? texMouseRight_ : texMouse_);
	}

	UpdateKeySpriteState(keyW_.get(), texW_, texWOutline_, input->PushKey(DIK_W));
	UpdateKeySpriteState(keyA_.get(), texA_, texAOutline_, input->PushKey(DIK_A));
	UpdateKeySpriteState(keyS_.get(), texS_, texSOutline_, input->PushKey(DIK_S));
	UpdateKeySpriteState(keyD_.get(), texD_, texDOutline_, input->PushKey(DIK_D));
	UpdateKeySpriteState(keySpace_.get(), texSpace_, texSpaceOutline_, input->PushKey(DIK_SPACE));

	if (keyW_) keyW_->Update();
	if (keyA_) keyA_->Update();
	if (keyS_) keyS_->Update();
	if (keyD_) keyD_->Update();
	if (labelMove_) labelMove_->Update();
	if (keySpace_) keySpace_->Update();
	if (labelDash_) labelDash_->Update();
	if (mouseIcon_) mouseIcon_->Update();
	if (labelAim_) labelAim_->Update();
	if (mouseShootIcon_) mouseShootIcon_->Update();
	if (labelShoot_) labelShoot_->Update();
}

void GuideUI::Draw() {
	if (!isActive_) {
		return;
	}
	if (keyW_) keyW_->Draw();
	if (keyA_) keyA_->Draw();
	if (keyS_) keyS_->Draw();
	if (keyD_) keyD_->Draw();
	if (labelMove_) labelMove_->Draw();
	if (keySpace_) keySpace_->Draw();
	if (labelDash_) labelDash_->Draw();
	if (mouseIcon_) mouseIcon_->Draw();
	if (labelAim_) labelAim_->Draw();
	if (mouseShootIcon_) mouseShootIcon_->Draw();
	if (labelShoot_) labelShoot_->Draw();
}
