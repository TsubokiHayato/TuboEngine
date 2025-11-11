#include "OverScene.h"
#include"OffScreenRendering.h"
#include "WinApp.h"
#include"Input.h"
#include"SceneManager.h"

namespace {
	// Smoothstep
	float SmoothStep(float t){ return t*t*(3.0f-2.0f*t); }
	// EaseOutBounce (0..1)
	float EaseOutBounce(float t){
		const float n1 = 7.5625f;
		const float d1 = 2.75f;
		if (t < 1.0f / d1) {
			return n1 * t * t;
		} else if (t < 2.0f / d1) {
			t -= 1.5f / d1; return n1 * t * t + 0.75f;
		} else if (t < 2.5f / d1) {
			t -= 2.25f / d1; return n1 * t * t + 0.9375f;
		} else {
			t -= 2.625f / d1; return n1 * t * t + 0.984375f;
		}
	}
}

void OverScene::Initialize() {
	// カメラの初期化
	camera = std::make_unique<Camera>();
	cameraTransform.rotate = {0.0f, 0.0f, 0.0f};      // カメラの回転を初期化
	cameraTransform.scale = {1.0f, 1.0f, 1.0f};       // カメラのスケールを初期化
	cameraTransform.translate = {0.0f, 0.0f, -15.0f}; // カメラの位置を初期化
	camera->SetTranslate(cameraTransform.translate);
	camera->setRotation(cameraTransform.rotate);
	camera->setScale(cameraTransform.scale);

	player = std::make_unique<Player>();
	player->Initialize();
	player->SetIsDontMove(true);
	player->SetCamera(camera.get());

	// アニメ開始位置へ
	player->SetPosition(startPos_);
	// 初期回転リセット
	player->SetRotation({0.0f, 0.0f, 0.0f});
	animTime_ = 0.0f;

	// GAME OVER 文字列（1文字ごとに個別テクスチャ: "G.png" など）
	const char* text = "GAMEOVER"; // 8文字
	letters_.clear();
	int count = 0; while (text[count] != '\0') ++count;
	float clientW = static_cast<float>(WinApp::GetInstance()->GetClientWidth());
	float totalW = count * letterSize_.x + (count - 1) * lettersGap_;
	float startX = (clientW - totalW) * 0.5f; // 中央寄せ
	float x = startX;
	for (int i=0; text[i] != '\0'; ++i){
		LetterAnim la;
		la.sprite = std::make_unique<Sprite>();
		// 文字に対応するテクスチャ "<prefix><Letter>.png" を使用
		std::string texPath = letterTexturePrefix_ + std::string(1, text[i]) + ".png";
		la.sprite->Initialize(texPath);
		la.sprite->SetSize(letterSize_);
		la.start = { x, lettersStartY_ };
		la.end   = { x, lettersRowY_ };
		la.delay = i * letterStagger_;
		la.time = 0.0f;
		la.sprite->SetPosition(la.start);
		letters_.push_back(std::move(la));
		x += letterSize_.x + lettersGap_;
	}

	
	restartSprite_ = std::make_unique<Sprite>();
	restartSprite_->Initialize("restart.png");
	restartSprite_->SetPosition({640.0f, 680.0f});
	restartSprite_->SetAnchorPoint({0.5f, 0.5f});
	restartSprite_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
}
void OverScene::Update() {
	// カメラ更新
	camera->SetTranslate(cameraTransform.translate);
	camera->setRotation(cameraTransform.rotate);
	camera->setScale(cameraTransform.scale);
	camera->Update();

	// 落下＋バウンスをイージングで
	const float dt = 1.0f/60.0f;
	animTime_ += dt;
	float t = animDuration_ > 0.0f ? std::min(animTime_/animDuration_, 1.0f) : 1.0f;
	// Yはバウンス、Xはスムースに補間
	float y0 = startPos_.y;
	float y1 = endPos_.y; // 0
	float by = EaseOutBounce(t); // 0..1
	float y = y0 + (y1 - y0) * by;
	// Xは少し寄せる（開始X→終了X+xOffset_）
	float ex = endPos_.x + xOffset_;
	float x = startPos_.x + (ex - startPos_.x) * SmoothStep(t);
	Vector3 pos = { x, y, endPos_.z };
	player->SetPosition(pos);

	// 横倒れ（Z軸回りに倒す）: tに合わせて0→目標角
	float tiltT = SmoothStep(t);
	Vector3 rot = player->GetRotation();
	rot.z = -tiltSign_ * tiltTargetRad_ * tiltT;
	player->SetRotation(rot);

	player->Update();

	// ループ動作（任意）
	if (loopBounce_ && t >= 1.0f){
		animTime_ = 0.0f;
		player->SetPosition(startPos_);
		player->SetRotation({0.0f,0.0f,0.0f});
	}

	// 文字アニメ（各文字を遅延つきで落とす）
	for (auto& l : letters_){
		l.time += dt;
		float tt = std::clamp((l.time - l.delay)/letterDuration_, 0.0f, 1.0f);
		float by2 = SmoothStep(tt); // 落下はスムーズ、必要ならBounceに変更
		Vector2 p = { l.start.x + (l.end.x - l.start.x) * by2,
					l.start.y + (l.end.y - l.start.y) * by2 };
		l.sprite->SetPosition(p);
		l.sprite->Update();
	}
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		SceneManager::GetInstance()->ChangeScene(STAGE);

	}


	restartSprite_->Update();
}

void OverScene::Finalize() {}

void OverScene::Object3DDraw() { player->Draw(); }

void OverScene::SpriteDraw() {
	for (auto& l : letters_) { l.sprite->Draw(); }
	restartSprite_->Draw();
}

void OverScene::ImGuiDraw() {

#ifdef USE_IMGUI
	ImGui::Begin("OVERScene");
	ImGui::Text("OVERScene");
	ImGui::Checkbox("Loop", &loopBounce_);
	ImGui::DragFloat("Duration", &animDuration_, 0.01f, 0.1f, 5.0f);
	ImGui::DragFloat3("StartPos", &startPos_.x, 0.05f);
	ImGui::DragFloat3("EndPos", &endPos_.x, 0.05f);
	ImGui::DragFloat("X Offset", &xOffset_, 0.01f, -5.0f, 5.0f);
	ImGui::DragFloat("TiltTargetRad", &tiltTargetRad_, 0.01f, 0.0f, 3.14f);
	ImGui::SliderInt("TiltSign (+Right/-Left)", &tiltSign_, -1, 1);

	ImGui::Separator();
	ImGui::Text("GAME OVER UI");
	ImGui::DragFloat("LetterDuration", &letterDuration_, 0.01f, 0.05f, 2.0f);
	ImGui::DragFloat("LetterStagger", &letterStagger_, 0.005f, 0.0f, 0.5f);
	ImGui::DragFloat2("LetterSize", &letterSize_.x, 1.0f, 8.0f, 512.0f);
	ImGui::DragFloat("LettersRowY", &lettersRowY_, 1.0f, -200.0f, 700.0f);
	ImGui::DragFloat("LettersStartY", &lettersStartY_, 1.0f, -700.0f, 0.0f);
	ImGui::DragFloat("LettersGap", &lettersGap_, 0.5f, 0.0f, 50.0f);
	if (ImGui::Button("Reset Letters")) {
		Initialize();
	}
	ImGui::End();

	ImGui::Begin("Camera");
	ImGui::DragFloat3("Camera Translate", &cameraTransform.translate.x, 0.1f);
	ImGui::DragFloat3("Camera Rotate", &cameraTransform.rotate.x, 0.01f);
	ImGui::DragFloat3("Camera Scale", &cameraTransform.scale.x, 0.01f);
	ImGui::End();
#endif // USE_IMGUI
}

void OverScene::ParticleDraw() {}
