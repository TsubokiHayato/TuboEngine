#include "PlayerStatusRingUI.h"
#include "TextureManager.h"
#include "WinApp.h"
#include "Matrix.h"
#include "application/Character/Player/Player.h"
#include "Camera.h"
#include <cmath>
#include <algorithm>

TuboEngine::Math::Vector2 PlayerStatusRingUI::WorldToScreen_(const TuboEngine::Math::Vector3& world, const Camera* cam) {
	if (!cam) return {0, 0};
	const TuboEngine::Math::Matrix4x4& vp = cam->GetViewProjectionMatrix();
	TuboEngine::Math::Vector3 v = TransformCoord(world, vp);
	int sw = (int)TuboEngine::WinApp::GetInstance()->GetClientWidth();
	int sh = (int)TuboEngine::WinApp::GetInstance()->GetClientHeight();
	TuboEngine::Math::Vector2 screen = {};
	screen.x = (v.x * 0.5f + 0.5f) * sw;
	screen.y = (-v.y * 0.5f + 0.5f) * sh;
	return screen;
}

float PlayerStatusRingUI::Clamp01_(float v) {
	return (v < 0.0f) ? 0.0f : (v > 1.0f) ? 1.0f : v;
}

void PlayerStatusRingUI::Initialize() {
	TextureManager::GetInstance()->LoadTexture(hpTex_);
	TextureManager::GetInstance()->LoadTexture(dodgeSegTex_);

	hpRing_ = std::make_unique<Sprite>();
	hpRing_->Initialize(hpTex_);
	hpRing_->SetAnchorPoint({0.5f, 0.5f});
	hpRing_->SetGetIsAdjustTextureSize(false);

	// Dodge segments
	dodgeSegments_.clear();
	dodgeSegments_.reserve(std::max(0, dodgeSegmentCount_));
	for (int i = 0; i < dodgeSegmentCount_; ++i) {
		auto s = std::make_unique<Sprite>();
		s->Initialize(dodgeSegTex_);
		s->SetAnchorPoint({0.5f, 0.5f});
		s->SetGetIsAdjustTextureSize(false);
		dodgeSegments_.push_back(std::move(s));
	}
}

void PlayerStatusRingUI::Update(const Player* player, const Camera* cam) {
	if (!player || !cam) return;
	if (!hpRing_) return;

	// プレイヤー中心の少し上に配置
	TuboEngine::Math::Vector3 playerWorldPosition = player->GetPosition();
	TuboEngine::Math::Vector2 screen = WorldToScreen_(playerWorldPosition, cam);
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

	// ---- Dodge ゲージ（セグメント円弧） ----
	// 「ダッシュが使える状態のときにゲージが溜まってる」= cooldown残が0なら満タン
	// cooldown中(dodgeCooldownTimer_>0)は 0→1 に向かって溜まる
	float cd = player->GetDodgeCooldownTimer();
	float cdDur = std::max(0.0001f, player->GetDodgeCooldownDuration());
	// cd=cdDur の瞬間(回避直後)は 0、cd=0 で 1
	float dodgeReadyRatio = 1.0f - (cd / cdDur);
	// 回避中は強制0（使用中として空に）
	if (player->GetIsDodging()) {
		dodgeReadyRatio = 0.0f;
	}
	dodgeReadyRatio = Clamp01_(dodgeReadyRatio);

	int visibleCount = static_cast<int>(std::round(dodgeReadyRatio * static_cast<float>(dodgeSegmentCount_)));
	visibleCount = std::clamp(visibleCount, 0, dodgeSegmentCount_);

	// 半径（中心からセグメントまでの距離）
	float radius = dodgeRingSizePx_ * 0.5f;
	float step = (dodgeSegmentCount_ > 0) ? (DirectX::XM_2PI / static_cast<float>(dodgeSegmentCount_)) : DirectX::XM_2PI;

	Vector4 segCol{0.6f, 0.9f, 1.0f, ringAlpha_};
	for (int i = 0; i < dodgeSegmentCount_; ++i) {
		if (i >= static_cast<int>(dodgeSegments_.size()) || !dodgeSegments_[i]) continue;
		auto* s = dodgeSegments_[i].get();

		// 時計回りに消す: i < visibleCount のものだけ表示
		// （開始は12時、そこから時計回り）
		bool on = (i < visibleCount);
		Vector4 c = segCol;
		c.w = on ? segCol.w : 0.0f;
		s->SetColor(c);

		float ang = dodgeStartAngleRad_ + step * static_cast<float>(i);
		float x = screen.x + std::cos(ang) * radius;
		float y = screen.y + std::sin(ang) * radius;
		s->SetPosition({std::round(x), std::round(y)});

		// セグメントを接線方向に向ける（円周に沿う）
		s->SetRotation(ang + DirectX::XM_PIDIV2);
		s->SetSize({dodgeSegmentLengthPx_, dodgeSegmentThicknessPx_});
		s->Update();
	}
}

void PlayerStatusRingUI::Draw() {
	//if (hpRing_) hpRing_->Draw();
	for (auto& s : dodgeSegments_) {
		if (s) s->Draw();
	}
}
