#include "TitleScene.h"
#include "SceneManager.h"
#include "Input.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "numbers"

void TitleScene::Initialize() {

	//カメラ
	camera = std::make_unique<Camera>();
	camera->SetTranslate({ 0.0f,0.0f,-5.0f });
	camera->setRotation({ 0.0f,0.0f,0.0f });
	camera->setScale({ 1.0f,1.0f,1.0f });

	// シーンチェンジアニメーション初期化（シーン開始時はDisappearingで覆いを消す）
	sceneChangeAnimation = std::make_unique<SceneChangeAnimation>(1280, 720, 80, 1.5f, "barrier.png");
	sceneChangeAnimation->Initialize();
	isRequestSceneChange = false;
}

void TitleScene::Update() {
	//カメラ
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);
	camera->Update();

	// スペースキーで覆いを出すリクエスト
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		sceneChangeAnimation->SetPhase(SceneChangeAnimation::Phase::Appearing); // 覆いを出す
		isRequestSceneChange = true;
	}

	sceneChangeAnimation->Update(1.0f / 60.0f);

	// アニメーションが終わったらシーン遷移
	// スペースキーで覆いを出すリクエスト
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		// アニメーション中は新たなアニメーションを開始しない
		if (sceneChangeAnimation->IsFinished()) {
			sceneChangeAnimation->SetPhase(SceneChangeAnimation::Phase::Appearing);
			isRequestSceneChange = true;
		}
	}
}

void TitleScene::Finalize() {


}

void TitleScene::Object3DDraw() {}

void TitleScene::SpriteDraw() {
	// ...既存のスプライト描画...

	// アニメーション描画
	if (sceneChangeAnimation) {
		sceneChangeAnimation->Draw();
	}
}

void TitleScene::ImGuiDraw() {

#ifdef USE_IMGUI
	//カメラ
	ImGui::Begin("camera");
	ImGui::DragFloat3("Position", &cameraPosition.x,0.01f);
	ImGui::DragFloat3("Rotation", &cameraRotation.x,0.01f);
	ImGui::DragFloat3("Scale", &cameraScale.x,0.01f);
	ImGui::End();

	if (sceneChangeAnimation) {
		sceneChangeAnimation->DrawImGui();
	}
#endif // USE_IMGUI
}

void TitleScene::ParticleDraw() {
	//パーティクル
	particle->Draw();

}
