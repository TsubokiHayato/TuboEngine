#include "DebugScene.h"
#include "BlendMode.h"
#include "ImGuiManager.h"
#include "ModelManager.h"
#include "SceneManager.h"
#include "TextureManager.h"
void DebugScene::Initialize() {

	// テクスチャマネージャに追加する画像ハンドル
	std::string uvCheckerTextureHandle = "uvChecker.png";
	std::string monsterBallTextureHandle = "monsterBall.png";

	std::string testDDSTextureHandle = "rostock_laage_airport_4k.dds";

	// モデルファイルパス
	const std::string modelFileNamePath = "sphere.gltf";
	// モデルファイルパス2
	const std::string modelFileNamePath2 = "terrain.obj";

	// オーディオ
	const std::string audioFileName = "fanfare.wav";

#pragma region Audioの初期化
	audio = std::make_unique<Audio>();
	audio->Initialize(audioFileName);
	//audio->Play(false);

#pragma endregion Audioの初期化


	skyBox = std::make_unique<SkyBox>();
	skyBox->Initialize(testDDSTextureHandle);

	

#pragma region cameraの初期化
	// カメラ

	camera = std::make_unique<Camera>();
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);

#pragma endregion cameraの初期化

	// シーンチェンジアニメーション
	sceneChangeAnimation = std::make_unique<SceneChangeAnimation>(1280, 720, 80, 1.5f, "barrier.png");
	sceneChangeAnimation->Initialize(); 
}

void DebugScene::Update() {

	// --- 既存のカメラ・オブジェクト・スプライト更新処理 ---
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);
	camera->Update();


	skyBox->SetCamera(camera.get());
	skyBox->Update();

	// スペースキーで覆いを出すリクエスト
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		// アニメーション中は新たなアニメーションを開始しない
		if (sceneChangeAnimation->IsFinished()) {
			sceneChangeAnimation->SetPhase(SceneChangeAnimation::Phase::Appearing);
			isRequestSceneChange = true;
		}
	}

	sceneChangeAnimation->Update(1.0f / 60.0f);

	// アニメーションが終わったらシーン遷移
	if (isRequestSceneChange && sceneChangeAnimation->IsFinished()) {
		// ここでシーン遷移処理を呼ぶ
		SceneManager::GetInstance()->ChangeScene(SCENE::TITLE);
		isRequestSceneChange = false; // フラグリセット
	}
}

void DebugScene::Finalize() {

}

void DebugScene::Object3DDraw() {
	
	skyBox->Draw();
}

void DebugScene::SpriteDraw() {
	

	// アニメーション描画

	sceneChangeAnimation->Draw();
}

void DebugScene::ImGuiDraw() {

#ifdef USE_IMGUI
	ImGui::Begin("DebugScene");
	ImGui::Text("Hello, DebugScene!");

	ImGui::End();

	// skyBoxのImGui

	static Vector3 skyBoxPosition = skyBox->GetTransform().translate;
	static Vector3 skyBoxRotation = skyBox->GetTransform().rotate;
	static Vector3 skyBoxScale = skyBox->GetTransform().scale;

	ImGui::Begin("SkyBox");
	ImGui::DragFloat3("Position", &skyBoxPosition.x, 0.1f);
	ImGui::DragFloat3("Rotation", &skyBoxRotation.x, 0.1f);
	ImGui::DragFloat3("Scale", &skyBoxScale.x, 0.1f);
	ImGui::End();

#ifdef _DEBUG

	ImGui::Begin("camera");
	ImGui::DragFloat3("Position", &cameraPosition.x, 0.1f);
	ImGui::DragFloat3("Rotation", &cameraRotation.x, 0.1f);
	ImGui::DragFloat3("Scale", &cameraScale.x, 0.1f);
	Matrix4x4 projectionInverse = Inverse(camera->GetProjectionMatrix());
	ImGui::Text("Projection Inverse:");
	ImGui::Text("m[0][0]: %f", projectionInverse.m[0][0]);
	ImGui::Text("m[1][1]: %f", projectionInverse.m[1][1]);
	ImGui::Text("m[2][2]: %f", projectionInverse.m[2][2]);
	ImGui::Text("m[3][3]: %f", projectionInverse.m[3][3]);
	ImGui::Text("m[0][1]: %f", projectionInverse.m[0][1]);
	ImGui::Text("m[1][0]: %f", projectionInverse.m[1][0]);

	ImGui::End();

	

	static float scratchPosition = 0.0f;
	static bool isScratching = false;
	static float lastScratchPosition = 0.0f;
	// 再生時間
	float duration = audio->GetSoundDuration();

	ImGui::Begin("Audio Control");

	if (ImGui::Button("Play")) {
		audio->Play(false);
	}
	if (ImGui::Button("Stop")) {
		audio->Stop();
	}
	if (ImGui::Button("Pause")) {
		audio->Pause();
	}
	if (ImGui::Button("Resume")) {
		audio->Resume();
	}
	// volume
	static float volume = 0.1f;
	ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f);
	audio->SetVolume(volume);

	// 再生バー
	static float playbackPosition = 0.0f;
	// 再生位置の取得
	playbackPosition = audio->GetPlaybackPosition();
	// 再生位置の視認
	ImGui::SliderFloat("Playback Position", &playbackPosition, 0.0f, duration);
	// audio->SetPlaybackPosition(playbackPosition);

	// speed
	static float speed = 1.0f;
	ImGui::SliderFloat("Speed", &speed, 0.0f, 2.0f);
	audio->SetPlaybackSpeed(speed);

	ImGui::End();

#endif // DEBUG

	sceneChangeAnimation->DrawImGui();

	#endif // USE_IMGUI	
}

void DebugScene::ParticleDraw() {}
