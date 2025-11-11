#include "GameOverState.h"
#include "Animation/SceneChangeAnimation.h"
#include "Character/Player/Player.h"
#include "LineManager.h"
#include "StageScene.h"
#include "engine/graphic/3d/Object3d.h"
#include <algorithm>
#include <cmath>

/// GameOverState ///
void GameOverState::Enter(StageScene* scene) {
	player_ = scene->GetPlayer();
	elapsed_ = 0.0f;
	phase_ = Phase ::None;
	currentAlpha_ = 1.0f;
	dizzyTime_ = 0.0f;
	dizzyAlpha_ = 1.0f;
	fallDir_ = {0.0f, 0.0f, 0.0f};
	initialYaw_ = player_ ? player_->GetRotation().z : 0.0f;
	camAnimTime_ = 0.0f;

	std::string testDDSTextureHandle = "rostock_laage_airport_4k.dds";
	TextureManager::GetInstance()->LoadTexture(testDDSTextureHandle);

	// プレイヤー操作を止める
	if (player_) {
		player_->Initialize();
		player_->SetPosition({});
		player_->SetIsDontMove(true);
		player_->SetModelAlpha(1.0f);
		player_->SetScale({1.0f, 1.0f, 1.0f});
		player_->SetRotation({5.0f, 3.14f, 3.14f});
	}

	// フォローカメラ初期化（プレイヤー追従）
	scene->GetFollowCamera()->Initialize(player_, Vector3{0.0f, 0.0f, 70.0f}, 0.25f);
	scene->GetFollowCamera()->Update();

	player_->SetCamera(scene->GetFollowCamera()->GetCamera());
	player_->Update();

	// カメラの現在値をスタート値として取得（初期化しない）
	if (auto* fc = scene->GetFollowCamera()) {
		camStartOffset_ = fc->GetOffset();
		camStartRot_ = fc->GetRotation();
	}

	// 目が回る演出用の簡易3Dアイコンを作成（小さな球）
	dizzy1_ = std::make_unique<Object3d>();
	dizzy1_->Initialize("star.obj");

	dizzy1_->SetScale({0.3f, 0.3f, 0.3f});
	if (scene->GetFollowCamera())
		dizzy1_->SetCamera(scene->GetFollowCamera()->GetCamera());
	dizzy1_->SetModelColor({1.0f, 1.0f, 0.2f, 1.0f});
	dizzy1_->Update();

	dizzy2_ = std::make_unique<Object3d>();
	dizzy2_->Initialize("star.obj");
	dizzy2_->SetScale({0.3f, 0.3f, 0.3f});
	if (scene->GetFollowCamera())
		dizzy2_->SetCamera(scene->GetFollowCamera()->GetCamera());
	dizzy2_->SetModelColor({1.0f, 1.0f, 0.2f, 1.0f});
	dizzy2_->Update();

	// 暗転アニメーションの準備
	TextureManager::GetInstance()->LoadTexture("barrier.png");
	blackoutSprite_ = std::make_unique<Sprite>();
	blackoutSprite_->Initialize("barrier.png");
	blackoutSprite_->SetSize({1280.0f, 720.0f});
	blackoutSprite_->SetColor({0.0f, 0.0f, 0.0f, 0.0f});
	blackoutSprite_->Update();

	scene->GetSkyDome()->Initialize();
	scene->GetSkyDome()->SetCamera(scene->GetFollowCamera()->GetCamera());
	scene->GetSkyDome()->Update();

	restartSprite_ = std::make_unique<Sprite>();
	restartSprite_->Initialize("SpaceToStart.png");
	restartSprite_->SetPosition({640.0f, 680.0f});
	restartSprite_->SetAnchorPoint({0.5f, 0.5f});
	restartSprite_->Update();
}

static float SmoothStep(float t) {
	// 3次のスムースステップ
	return t * t * (3.0f - 2.0f * t);
}

static Vector3 Lerp(const Vector3& a, const Vector3& b, float t) { return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t}; }

void GameOverState::Update(StageScene* scene) {
	constexpr float dt = 1.0f / 60.0f;
	elapsed_ += dt;

	if (phase_ == Phase::None) {

		phase_ = Phase::CameraMove;

		return;
	}

	// カメラ移動が終わるまでは演出停止
	if (phase_ == Phase::CameraMove) {
		// カメラをイージングで目標へ
		if (auto* fc = scene->GetFollowCamera()) {
			camAnimTime_ = std::min(camAnimTime_ + dt, camAnimDuration_);
			float t = camAnimDuration_ > 0.0f ? camAnimTime_ / camAnimDuration_ : 1.0f;
			float te = SmoothStep(t);
			Vector3 newOffset = Lerp(camStartOffset_, camTargetOffset_, te);
			Vector3 newRot = Lerp(camStartRot_, camTargetRot_, te);
			fc->SetOffset(newOffset);
			fc->SetRotation(newRot);
			fc->Update();
			LineManager::GetInstance()->SetDefaultCamera(fc->GetCamera());
		}
		if (camAnimTime_ >= camAnimDuration_) {
			phase_ = Phase::Dizzy;
			// リセット
			dizzyTime_ = 0.0f;
		}
		// プレイヤーは待機更新のみ
		if (scene->GetFollowCamera())
			scene->GetFollowCamera()->Update();
		player_->SetCamera(scene->GetFollowCamera()->GetCamera());
		player_->Update();

		scene->GetSkyDome()->SetCamera(scene->GetFollowCamera()->GetCamera());
		scene->GetSkyDome()->Update();
		return;
	}

	if (phase_ == Phase::Dizzy) {
		if (player_) {
			Vector3 rot = player_->GetRotation();
			rot.z += spinSpeed_ * 1.5f * dt;
			player_->SetRotation(rot);
		}
		dizzyTime_ += dt;
		float angle = orbitSpeed_ * dizzyTime_;
		Vector3 center = player_->GetPosition();
		Vector3 p1 = center;
		p1.x += std::cos(angle) * orbitRadius_;
		p1.y += std::sin(angle) * orbitRadius_;
		p1.z += orbitHeightZ_;
		Vector3 p2 = center;
		p2.x += std::cos(angle + 3.14159265f) * orbitRadius_ * 0.8f;
		p2.y += std::sin(angle + 3.14159265f) * orbitRadius_ * 0.8f;
		p2.z += orbitHeightZ_;

		dizzyAlpha_ = (std::max)(0.0f, 1.0f - (dizzyTime_ / dizzyDuration_));
		{
			Vector4 c = dizzy1_->GetModelColor();
			c.w = dizzyAlpha_;
			dizzy1_->SetModelColor(c);
			c = dizzy2_->GetModelColor();
			c.w = dizzyAlpha_;
			dizzy2_->SetModelColor(c);
		}

		dizzy1_->SetPosition(p1);
		dizzy1_->SetRotation({0, 0, angle});
		dizzy1_->Update();
		dizzy2_->SetPosition(p2);
		dizzy2_->SetRotation({0, 0, -angle});
		dizzy2_->Update();

		if (dizzyTime_ >= dizzyDuration_) {
			phase_ = Phase::Blackout;
		}
	} else if (phase_ == Phase::Blackout) {
		// staticをやめ、スプライトの現在アルファを元に進行度を管理する
		float alpha = blackoutSprite_->GetColor().w;
		const float maxAlpha = 1.0f;
		alpha += 0.01f;
		blackoutSprite_->SetColor({0.0f, 0.0f, 0.0f, alpha});
		blackoutSprite_->Update();
		if (alpha >= maxAlpha) {
			phase_ = Phase::Done;
			scene->ChangeNextScene(OVER);
		}
	}

	// プレイヤー自身も更新（描画のため）
	if (scene->GetFollowCamera())
		scene->GetFollowCamera()->Update();
	player_->SetCamera(scene->GetFollowCamera()->GetCamera());
	player_->Update();

	scene->GetSkyDome()->SetCamera(scene->GetFollowCamera()->GetCamera());
	scene->GetSkyDome()->Update();
	restartSprite_->Update();
}

void GameOverState::Exit(StageScene* scene) {}

void GameOverState::Object3DDraw(StageScene* scene) {
	scene->GetSkyDome()->Draw();
	player_->Draw();
	if (phase_ == Phase::Dizzy) {
		if (dizzy1_)
			dizzy1_->Draw();
		if (dizzy2_)
			dizzy2_->Draw();
	}
}

void GameOverState::SpriteDraw(StageScene* scene) {

	if (phase_ == Phase::Blackout || phase_ == Phase::Done) {
		blackoutSprite_->Draw();
	}
	if (phase_ == Phase::None) {
		restartSprite_->Draw();
	}
}

void GameOverState::ImGuiDraw(StageScene* scene) {

#ifdef USE_IMGUI
	// alpha
	ImGui::Begin("Sprite Draw");
	ImGui::Text("blackOutAlpha: %.2f", blackoutSprite_->GetColor().w);
	ImGui::End();

	scene->GetSkyDome()->DrawImGui();

	// playerRotation
	ImGui::Begin("Player Rotation");
	ImGui::Text("Player Rotation Z: %.2f", player_->GetRotation().z);
	Vector3 rot = player_->GetRotation();
	ImGui::DragFloat3("Rotation", &rot.x, 0.1f);
	player_->SetRotation(rot);

	ImGui::End();

	if (scene->GetFollowCamera())
		scene->GetFollowCamera()->DrawImGui();

	if (ImGui::CollapsingHeader("GameOver Animation")) {
		ImGui::SliderFloat("SpinSpeed", &spinSpeed_, 0.0f, 10.0f, "%.2f");
		ImGui::SliderFloat("ShrinkSpeed", &shrinkSpeed_, 0.0f, 2.0f, "%.2f");
		ImGui::SliderFloat("FadeSpeed", &fadeSpeed_, 0.0f, 3.0f, "%.2f");
		ImGui::SliderFloat("WobbleAmp", &wobbleAmp_, 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("WobbleDecay", &wobbleDecay_, 0.0f, 3.0f, "%.2f");
		ImGui::SliderFloat("WobbleFreq", &wobbleFreq_, 0.0f, 12.0f, "%.2f");
		ImGui::SliderFloat("TiltTarget", &tiltTarget_, 0.0f, 2.0f, "%.2f");
		ImGui::SliderFloat("TiltSpeed", &tiltSpeed_, 0.0f, 3.0f, "%.2f");
		ImGui::SliderFloat("FallMoveSpeed", &fallMoveSpeed_, 0.0f, 2.0f, "%.2f");
		ImGui::SliderFloat("DizzyDuration", &dizzyDuration_, 0.2f, 3.0f, "%.2f");
		ImGui::SliderFloat("OrbitSpeed", &orbitSpeed_, 0.0f, 20.0f, "%.2f");
		ImGui::SliderFloat3("CamTargetOffset", &camTargetOffset_.x, -50.0f, 50.0f);
		ImGui::SliderFloat3("CamTargetRot", &camTargetRot_.x, -3.2f, 3.2f);
		ImGui::SliderFloat("CamAnimDuration", &camAnimDuration_, 0.0f, 3.0f, "%.2f");
	}
#endif // USE_IMGUI
}

void GameOverState::ParticleDraw(StageScene* scene) {}
