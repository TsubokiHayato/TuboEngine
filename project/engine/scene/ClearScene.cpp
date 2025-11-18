#include "ClearScene.h"
#include "Character/Player/Player.h"
#include "Sprite.h"
#include <cmath>
#include <format>
#include "TextureManager.h"
#include "WinApp.h"
#include "externals/imgui/imgui.h"
#include "Input.h"
#include "SceneManager.h" // 追加：シーン遷移呼び出し用

static float Clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }

// イージング関数（軽いイーズアウト）
static float EaseOutQuad(float t) { return 1.0f - (1.0f - t) * (1.0f - t); }
// 補助 Lerp
static Vector3 LerpVec(const Vector3& a, const Vector3& b, float t) {
	return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t };
}

void ClearScene::Initialize() {
	// カメラの初期化
	camera = std::make_unique<Camera>();
	cameraTransform.rotate = {0.0f, 0.0f, 0.0f};
	cameraTransform.scale = {1.0f, 1.0f, 1.0f};
	cameraTransform.translate = {0.0f, 0.0f, -15.0f};
	camera->SetTranslate(cameraTransform.translate);
	camera->setRotation(cameraTransform.rotate);
	camera->setScale(cameraTransform.scale);

	// プレイヤー初期化
	player_ = std::make_unique<Player>();
	player_->Initialize();
	// シーン側から操作するため入力は無効化（自動演出のみ）
	player_->SetIsDontMove(true);
	// カメラをセット（描画用）
	player_->SetCamera(camera.get());
	// 初期位置を手前に配置して見やすく
	player_->SetPosition({0.0f, 0.0f, 4.0f});
	player_->SetScale({1.0f, 1.0f, 1.0f});
	player_->Update();

	// 画面中心を取得して配置する
	float screenW = static_cast<float>(WinApp::GetInstance()->GetClientWidth());
	float screenH = static_cast<float>(WinApp::GetInstance()->GetClientHeight());

	// CLEAR 文字スプライトの生成
	const char letters[5] = { 'C','L','E','A','R' };

	letterSprites_.clear();
	letterBaseSizes_.clear();
	letterTextureNames_.clear();

	for (int i = 0; i < 5; ++i) {
		auto s = std::make_unique<Sprite>();
		std::string tex = std::format("{}.png", letters[i]); // 例: "C.png"
		letterTextureNames_.push_back(tex);

		TextureManager::GetInstance()->LoadTexture(tex);
		const auto& meta = TextureManager::GetInstance()->GetMetaData(tex);
		Vector2 texSize = { static_cast<float>(meta.width), static_cast<float>(meta.height) };

		s->Initialize(tex);

		// アンカー中央・位置を画面中心基準に設定（yは後で letterYOffset_ で調整）
		s->SetAnchorPoint({0.5f, 0.5f});
		float x = screenW * 0.5f + (i - 2) * letterSpacing_;
		float y = screenH * 0.25f + letterYOffset_;
		s->SetPosition({ x, y });

		// 初期サイズ（ピクセル）をセットして保存
		const float initialScale = 0.9f;
		Vector2 initSize = { texSize.x * initialScale, texSize.y * initialScale };
		s->SetSize(initSize);
		s->Update();

		letterBaseSizes_.push_back(initSize);
		letterSprites_.push_back(std::move(s));
	}

	// シーンチェンジアニメーション初期化
	sceneChangeAnimation_ = std::make_unique<SceneChangeAnimation>(1280, 720, 80, 1.5f, "barrier.png");
	sceneChangeAnimation_->Initialize();

	// スペースアニメ初期値
	spaceAnimActive_ = false;
	spaceAnimTimer_ = 0.0f;
	spaceLaunchRight_ = true; // 初回は右へ

	isRequestSceneChange_ = false;

	elapsed_ = 0.0f;

	restartSprite_ = std::make_unique<Sprite>();
	restartSprite_->Initialize("restart.png");
	restartSprite_->SetPosition({640.0f, 680.0f});
	restartSprite_->SetAnchorPoint({0.5f, 0.5f});
	restartSprite_->Update();

}

void ClearScene::Update() {
	const float delta = 1.0f / 60.0f; // 必要なら実時間差分で置き換える
	elapsed_ += delta;

	// 入力：スペースでアニメ開始（アニメ中は無視）
	if (!spaceAnimActive_ && Input::GetInstance()->PushKey(DIK_SPACE)) {
		spaceAnimActive_ = true;
		spaceAnimTimer_ = 0.0f;
		// 元の値を保存
		if (player_) {
			spaceOrigPos_ = player_->GetPosition();
			spaceOrigRot_ = player_->GetRotation();
			spaceOrigScale_ = player_->GetScale();
			// 交互に左右へ飛ばす（ユーモアの演出）
			spaceLaunchRight_ = !spaceLaunchRight_;
		}
	}

	// プレイヤーに自動演出（ボブ：上下に揺れる）とスペースアニメの適用
	if (player_) {
		Vector3 pos = player_->GetPosition();
		Vector3 rot = player_->GetRotation();
		Vector3 scl = player_->GetScale();

		// ベースのゆらぎ（常時）
		const float bobAmp = 0.6f;
		const float bobSpeed = 1.8f;
		pos.y = std::sin(elapsed_ * bobSpeed) * bobAmp + spaceOrigPos_.y;

		// スペースアニメ中は画面外へ飛ばす一貫した挙動
		if (spaceAnimActive_) {
			spaceAnimTimer_ += delta;
			float t = Clamp01(spaceAnimTimer_ / spaceAnimDuration_);
			// 準備（0-0.18）：スクワッシュしてためる
			if (t < 0.18f) {
				float u = t / 0.18f;
				float squash = 1.0f - 0.35f * u; // 少し潰す
				scl = spaceOrigScale_;
				scl.x *= squash;
				scl.y *= (1.0f + (1.0f - squash) * 0.8f);
				rot.z = spaceOrigRot_.z + 0.6f * std::sin(u * 3.14f);
				pos.y = spaceOrigPos_.y - 0.15f * u; // ため差分
			}
			// ローンチ（0.18 - 0.6）：放物線的に大きく飛ばす（横方向に強く）
			else if (t < 0.6f) {
				float u = (t - 0.18f) / (0.6f - 0.18f);
				float ease = EaseOutQuad(u);
				// 横方向（左右）、大きめの横移動と上方向を付与、カメラの前後方向へも移動
				float lateral = (spaceLaunchRight_ ? 1.0f : -1.0f) * (30.0f + 20.0f * u); // だんだん加速
				float upward = ease * (spaceJumpHeight_ * 6.0f); // 大きく上がる
				float forward = -ease * 50.0f; // カメラ手前から飛び出して奥へ
				Vector3 target = { spaceOrigPos_.x + lateral, spaceOrigPos_.y + upward, spaceOrigPos_.z + forward };
				pos = LerpVec(spaceOrigPos_, target, ease);
				rot.z = spaceOrigRot_.z + 6.0f * ease; // くるくる回る
				// 少し伸びる
				scl = spaceOrigScale_;
				scl.y *= 1.0f + 0.6f * ease;
				scl.x *= 1.0f - 0.25f * ease;
			}
			// フェード＆消失（0.6 - 1.0）：回転は継続しながらフェードアウトして画面外へ加速
			else if (t < 1.0f) {
				float u = (t - 0.6f) / (1.0f - 0.6f);
				float ease = EaseOutQuad(u);
				// 現在位置を基準にさらに遠くへ押し出す
				float extraLat = (spaceLaunchRight_ ? 1.0f : -1.0f) * (60.0f * ease);
				float extraForward = -80.0f * ease;
				pos.x += extraLat;
				pos.z += extraForward;
				rot.z += 12.0f * ease;
				// 徐々に小さくして消える印象
				scl *= 1.0f - 0.6f * ease;
				// モデルのアルファを下げる（存在感フェード）
				float alpha = 1.0f - ease;
				player_->SetModelAlpha(alpha);
			}
			else {
				// 完全に画面外に到達した扱いにする
				// 位置を極端に遠ざけて見えないようにする
				Vector3 farPos = pos;
				farPos.x += (spaceLaunchRight_ ? 1.0f : -1.0f) * 200.0f;
				farPos.z -= 300.0f;
				player_->SetPosition(farPos);
				player_->SetModelAlpha(0.0f);
				// アニメ終了
				spaceAnimActive_ = false;
				spaceAnimTimer_ = 0.0f;

				// --- ここでシーンチェンジアニメーションを開始する ---
				if (sceneChangeAnimation_ && sceneChangeAnimation_->IsFinished()) {
					sceneChangeAnimation_->SetPhase(SceneChangeAnimation::Phase::Appearing);
					isRequestSceneChange_ = true;
				}
			}
		}
		else {
			// 非アニメ時の短い前進（短いズームインの印象）
			const float forwardStartZ = 4.0f;
			const float forwardEndZ = 2.0f;
			const float moveDuration = 1.8f;
			float t2 = Clamp01(elapsed_ / moveDuration);
			pos.z = forwardStartZ + (forwardEndZ - forwardStartZ) * t2;
			rot.z = 0.12f * std::sin(elapsed_ * 1.5f);
			scl = player_->GetScale(); // そのまま
			// モデルは表示を確実にしておく
			player_->SetModelAlpha(1.0f);
		}

		player_->SetPosition(pos);
		player_->SetRotation(rot);
		player_->SetScale(scl);
		player_->Update();
	}

	// カメラ更新
	camera->SetTranslate(cameraTransform.translate);
	camera->setRotation(cameraTransform.rotate);
	camera->setScale(cameraTransform.scale);
	camera->Update();

	// SceneChangeAnimation 更新
	if (sceneChangeAnimation_) {
		sceneChangeAnimation_->Update(delta);
	}

	// シーンチェンジアニメ完了でシーン遷移
	if (isRequestSceneChange_ && sceneChangeAnimation_ && sceneChangeAnimation_->IsFinished()) {
		SceneManager::GetInstance()->ChangeScene(SCENE::TITLE);
		isRequestSceneChange_ = false;
	}

	// 文字のフェード＆ポップ演出（位置・サイズは ImGui からも変更可能）
	for (size_t i = 0; i < letterSprites_.size(); ++i) {
		float startTime = i * letterDelay_;
		float localT = Clamp01((elapsed_ - startTime) / fadeDuration_);
		float t = 1.0f - std::pow(1.0f - localT, 2.0f);
		float scale = 0.6f + 0.4f * t;

		// base size はピクセル単位。アニメーション時は掛け算する
		Vector2 base = letterBaseSizes_[i];
		letterSprites_[i]->SetSize({ base.x * scale, base.y * scale });

		letterSprites_[i]->SetColor({ 1.0f, 1.0f, 1.0f, t });
		letterSprites_[i]->Update();
	}

	// UIは毎フレーム更新
	restartSprite_->Update();

}

void ClearScene::Finalize() {}

void ClearScene::Object3DDraw() {
	if (player_) player_->Draw();
}

void ClearScene::SpriteDraw() {
	for (size_t i = 0; i < letterSprites_.size(); ++i) {
		letterSprites_[i]->Draw();
	}

	restartSprite_->Draw();

	// シーンチェンジアニメーション描画
	if (sceneChangeAnimation_) {
		sceneChangeAnimation_->Draw();
	}
}

void ClearScene::ImGuiDraw() {

#ifdef USE_IMGUI
	ImGui::Begin("ClearScene");
	ImGui::Text("Clear Scene");
	ImGui::Text("Elapsed: %.2f", elapsed_);
	ImGui::End();

	ImGui::Begin("Camera");
	ImGui::DragFloat3("Camera Translate", &cameraTransform.translate.x, 0.1f);
	ImGui::DragFloat3("Camera Rotate", &cameraTransform.rotate.x, 0.01f);
	ImGui::DragFloat3("Camera Scale", &cameraTransform.scale.x, 0.01f);
	ImGui::End();

	ImGui::Begin("Sprite Letters");

	// 全体パラメータ
	if (ImGui::DragFloat("Letter Spacing", &letterSpacing_, 1.0f, 10.0f, 400.0f)) {
		// 位置を再計算
		float screenW = static_cast<float>(WinApp::GetInstance()->GetClientWidth());
		for (int i = 0; i < static_cast<int>(letterSprites_.size()); ++i) {
			float x = screenW * 0.5f + (i - 2) * letterSpacing_;
			auto pos = letterSprites_[i]->GetPosition();
			letterSprites_[i]->SetPosition({ x, pos.y });
			letterSprites_[i]->Update();
		}
	}
	if (ImGui::DragFloat("Letter Y Offset", &letterYOffset_, 1.0f, -600.0f, 600.0f)) {
		float screenH = static_cast<float>(WinApp::GetInstance()->GetClientHeight());
		for (size_t i = 0; i < letterSprites_.size(); ++i) {
			auto pos = letterSprites_[i]->GetPosition();
			float y = screenH * 0.5f + letterYOffset_;
			letterSprites_[i]->SetPosition({ pos.x, y });
			letterSprites_[i]->Update();
		}
	}
	ImGui::DragFloat("Letter Delay", &letterDelay_, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Fade Duration", &fadeDuration_, 0.01f, 0.01f, 3.0f);

	ImGui::Separator();
	if (ImGui::Button("Reset Positions")) {
		float screenW = static_cast<float>(WinApp::GetInstance()->GetClientWidth());
		float screenH = static_cast<float>(WinApp::GetInstance()->GetClientHeight());
		for (int i = 0; i < static_cast<int>(letterSprites_.size()); ++i) {
			float x = screenW * 0.5f + (i - 2) * letterSpacing_;
			float y = screenH * 0.5f + letterYOffset_;
			letterSprites_[i]->SetPosition({ x, y });
			letterSprites_[i]->SetSize(letterBaseSizes_[i]);
			letterSprites_[i]->SetColor({ 1.0f,1.0f,1.0f,0.0f });
			letterSprites_[i]->Update();
		}
	}

	ImGui::Separator();

	for (size_t i = 0; i < letterSprites_.size(); ++i) {
		std::string header = std::format("Letter {} ({})", static_cast<int>(i), letterTextureNames_[i]);
		if (ImGui::CollapsingHeader(header.c_str())) {
			// Position
			Vector2 pos = letterSprites_[i]->GetPosition();
			if (ImGui::DragFloat2(std::format("Position##{}", i).c_str(), &pos.x, 1.0f)) {
				letterSprites_[i]->SetPosition(pos);
				letterSprites_[i]->Update();
			}
			// Size
			Vector2 sz = letterSprites_[i]->GetSize();
			if (ImGui::DragFloat2(std::format("Size##{}", i).c_str(), &sz.x, 1.0f)) {
				letterSprites_[i]->SetSize(sz);
				letterSprites_[i]->Update();
			}
			// Color / Alpha
			Vector4 col = letterSprites_[i]->GetColor();
			float colf[4] = { col.x, col.y, col.z, col.w };
			if (ImGui::ColorEdit4(std::format("Color##{}", i).c_str(), colf)) {
				letterSprites_[i]->SetColor({ colf[0], colf[1], colf[2], colf[3] });
				letterSprites_[i]->Update();
			}
			// Anchor
			Vector2 anchor = letterSprites_[i]->GetAnchorPoint();
			if (ImGui::DragFloat2(std::format("Anchor##{}", i).c_str(), &anchor.x, 0.01f, 0.0f, 1.0f)) {
				letterSprites_[i]->SetAnchorPoint(anchor);
				letterSprites_[i]->Update();
			}
			// Flip X/Y
			bool flipX = letterSprites_[i]->GetFlipX();
			if (ImGui::Checkbox(std::format("FlipX##{}", i).c_str(), &flipX)) {
				letterSprites_[i]->SetFlipX(flipX);
				letterSprites_[i]->Update();
			}
			bool flipY = letterSprites_[i]->GetFlipY();
			if (ImGui::Checkbox(std::format("FlipY##{}", i).c_str(), &flipY)) {
				letterSprites_[i]->SetFlipY(flipY);
				letterSprites_[i]->Update();
			}
			ImGui::Text("Base Size: %.0f x %.0f", letterBaseSizes_[i].x, letterBaseSizes_[i].y);
			ImGui::Text("Texture: %s", letterTextureNames_[i].c_str());
			ImGui::Separator();
		}
	}

	ImGui::End();

	// プレイヤー情報表示
	if (player_) {
		ImGui::Begin("Player (ClearScene)");
		Vector3 ppos = player_->GetPosition();
		Vector3 prot = player_->GetRotation();
		Vector3 pscale = player_->GetScale();
		ImGui::Text("Position: %.2f, %.2f, %.2f", ppos.x, ppos.y, ppos.z);
		ImGui::Text("Rotation: %.2f, %.2f, %.2f", prot.x, prot.y, prot.z);
		ImGui::Text("Scale: %.2f, %.2f, %.2f", pscale.x, pscale.y, pscale.z);
		ImGui::Separator();

		if (ImGui::Button("Trigger Space Jump")) {
			// デバッグから強制発火
			spaceAnimActive_ = true;
			spaceAnimTimer_ = 0.0f;
			spaceOrigPos_ = player_->GetPosition();
			spaceOrigRot_ = player_->GetRotation();
			spaceOrigScale_ = player_->GetScale();
			spaceLaunchRight_ = !spaceLaunchRight_;
		}

		player_->DrawImGui();
		ImGui::End();
	}

#endif // USE_IMGUI
}

void ClearScene::ParticleDraw() {}
