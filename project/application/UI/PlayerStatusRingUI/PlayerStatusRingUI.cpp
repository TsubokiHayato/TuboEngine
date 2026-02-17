#include "PlayerStatusRingUI.h"

#include "TextureManager.h"
#include "WinApp.h"
#include "Matrix.h"

#include "application/Character/Player/Player.h"
#include "Camera.h"

#include <algorithm>
#include <cmath>

TuboEngine::Math::Vector2 PlayerStatusRingUI::WorldToScreen_(const TuboEngine::Math::Vector3& world, const Camera* cam) {
	if (!cam) return {0, 0};
	const TuboEngine::Math::Matrix4x4& vp = cam->GetViewProjectionMatrix();
	TuboEngine::Math::Vector3 v = TransformCoord(world, vp);
	int sw = (int)TuboEngine::WinApp::GetInstance()->GetClientWidth();
	int sh = (int)TuboEngine::WinApp::GetInstance()->GetClientHeight();
	TuboEngine::Math::Vector2 screen;
	screen.x = (v.x * 0.5f + 0.5f) * sw;
	screen.y = (-v.y * 0.5f + 0.5f) * sh;
	return screen;
}

float PlayerStatusRingUI::Clamp01_(float v) {
	return (v < 0.0f) ? 0.0f : (v > 1.0f) ? 1.0f : v;
}

void PlayerStatusRingUI::Initialize() {
	TextureManager::GetInstance()->LoadTexture(hpTex_);
	TextureManager::GetInstance()->LoadTexture(dodgeTex_);

	hpRing_ = std::make_unique<Sprite>();
	hpRing_->Initialize(hpTex_);
	hpRing_->SetAnchorPoint({0.5f, 0.5f});
	hpRing_->SetGetIsAdjustTextureSize(false);

	dodgeRing_ = std::make_unique<Sprite>();
	dodgeRing_->Initialize(dodgeTex_);
	dodgeRing_->SetAnchorPoint({0.5f, 0.5f});
	dodgeRing_->SetGetIsAdjustTextureSize(false);
}

void PlayerStatusRingUI::Update(const Player* player, const Camera* cam) {
	if (!player || !cam) return;
	if (!hpRing_ || !dodgeRing_) return;

	// プレイヤー中心の少し上に配置
	TuboEngine::Math::Vector3 wpos = player->GetPosition();
	TuboEngine::Math::Vector2 screen = WorldToScreen_(wpos, cam);
	screen = {std::round(screen.x + screenOffsetPx_.x), std::round(screen.y + screenOffsetPx_.y)};

	// ---- HP リング ----
	int hp = std::clamp(player->GetHP(), 0, assumedMaxHp_);
	float hpRatio = assumedMaxHp_ > 0 ? float(hp) / float(assumedMaxHp_) : 0.0f;
	hpRatio = Clamp01_(hpRatio);

	// 右方向から減っていくようにテクスチャを横方向に切り出す（簡易）
	// ※リング画像は「横方向に0..100%が並んでいる」前提。
	// もし単純な円画像なら、ここは常に全表示になります。
	{
		TuboEngine::Math::Vector2 fullSz = hpRing_->GetTextureSize();
		if (fullSz.x <= 0.0f || fullSz.y <= 0.0f) {
			hpRing_->AdjustTextureSize();
			fullSz = hpRing_->GetTextureSize();
		}
		hpRing_->SetTextureLeftTop({0.0f, 0.0f});
		hpRing_->SetTextureSize({fullSz.x * hpRatio, fullSz.y});
	}
	Vector4 hpCol{1.0f, 1.0f, 1.0f, ringAlpha_};
	hpRing_->SetColor(hpCol);
	hpRing_->SetPosition(screen);
	hpRing_->SetSize({hpRingSizePx_, hpRingSizePx_});
	hpRing_->Update();

	// ---- Dodge リング ----
	// 現状 Player にスタミナが無いので「回避クールダウンの残り」をリングにする
	// CanDodge() は private なので、見た目は isDodging_ / dodgeCooldownTimer_ が必要。
	// いったん「回避中なら0、そうでなければ1」で簡易表示。
	float dodgeReady = player->IsDashing() ? 0.0f : 1.0f; // IsDashingはダミー実装の可能性があるが、見た目用
	{
		TuboEngine::Math::Vector2 fullSz = dodgeRing_->GetTextureSize();
		if (fullSz.x <= 0.0f || fullSz.y <= 0.0f) {
			dodgeRing_->AdjustTextureSize();
			fullSz = dodgeRing_->GetTextureSize();
		}
		dodgeRing_->SetTextureLeftTop({0.0f, 0.0f});
		dodgeRing_->SetTextureSize({fullSz.x * dodgeReady, fullSz.y});
	}
	Vector4 dodgeCol{0.6f, 0.9f, 1.0f, ringAlpha_};
	dodgeRing_->SetColor(dodgeCol);
	dodgeRing_->SetPosition(screen);
	dodgeRing_->SetSize({dodgeRingSizePx_, dodgeRingSizePx_});
	dodgeRing_->Update();
}

void PlayerStatusRingUI::Draw() {
	if (hpRing_) hpRing_->Draw();
	if (dodgeRing_) dodgeRing_->Draw();
}
