#include "DebugScene.h"
#include"ImGuiManager.h"
#include"SceneManager.h"
#include"ModelManager.h"
#include"TextureManager.h"
#include"BlendMode.h"
void DebugScene::Initialize() {

	//テクスチャマネージャに追加する画像ハンドル
	std::string uvCheckerTextureHandle = "uvChecker.png";
	std::string monsterBallTextureHandle = "monsterBall.png";

	std::string testDDSTextureHandle = "rostock_laage_airport_4k.dds";

	//モデルファイルパス
	const std::string modelFileNamePath = "sphere.gltf";
	//モデルファイルパス2
	const std::string modelFileNamePath2 = "terrain.obj";


	//オーディオ
	const std::string audioFileName = "fanfare.wav";
	


#pragma region Audioの初期化
	audio = std::make_unique<Audio>();
	audio->Initialize(audioFileName);
	audio->Play(false);

#pragma endregion Audioの初期化
	/*---------------
		スプライト
	---------------*/
#pragma region スプライトの初期化
	// スプライト初期化

	for (uint32_t i = 0; i < 1; ++i) {

		Sprite* sprite = new Sprite();

		//もしfor文のiが偶数なら
		if (i % 2 == 0) {
			//モンスターボールを表示させる
			sprite->Initialize( monsterBallTextureHandle);
		} else {
			//uvCheckerを表示させる
			sprite->Initialize( uvCheckerTextureHandle);
		}


		// 各スプライトに異なる位置やプロパティを設定する
		//Vector2 spritePosition = { i * -1280.0f, 0.0f }; // スプライトごとに異なる位置
		Vector2 spritePosition = { 100.0f, 100.0f }; // スプライトごとに異なる位置
		float spriteRotation = 0.0f;                 // 回転は任意
		Vector4 spriteColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // 色は白（RGBA）
		Vector2 size = { 50.0f, 50.0f };             // 任意のサイズ

		
		//スプライトの位置や回転を設定
		sprite->SetPosition(spritePosition);
		sprite->SetRotation(spriteRotation);
		sprite->SetColor(spriteColor);
		sprite->SetSize(size);
		
		sprites.push_back(sprite);

	}
#pragma endregion スプライトの初期化


	skyBox = std::make_unique<SkyBox>();
	skyBox->Initialize(testDDSTextureHandle);


	/*---------------
	  オブジェクト3D
	---------------*/
#pragma region 3Dモデルの初期化
	//オブジェクト3D

	object3d = std::make_unique<Object3d>();
	object3d->Initialize(modelFileNamePath);

	object3d->SetModel(modelFileNamePath);
	object3d->SetCubeMapFilePath(skyBox->GetTextureFilePath());


	////////////////////////////////////////////////////////////////////////



	//オブジェクト3D

	object3d2 = std::make_unique<Object3d>();
	object3d2->Initialize(modelFileNamePath2);
	

#pragma endregion 3Dモデルの初期化

#pragma region cameraの初期化
	//カメラ

	camera = std::make_unique<Camera>();
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);

	object3d->SetCamera(camera.get());
	object3d2->SetCamera(camera.get());

#pragma endregion cameraの初期化


	//シーンチェンジアニメーション
	sceneChangeAnimation = std::make_unique<SceneChangeAnimation>(1280, 720, 80, 1.5f, "barrier.png");
	sceneChangeAnimation->Initialize();
	showSceneChangeAnimation = false;


}

void DebugScene::Update() {
	

	// --- 既存のカメラ・オブジェクト・スプライト更新処理 ---
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);
	camera->Update();

	object3d->Update();
	object3d->SetPosition(modelPosition);
	object3d->SetRotation(modelRotation);
	object3d->SetScale(modelScale);

	object3d2->Update();
	object3d2->SetPosition(modelPosition2);
	object3d2->SetRotation(modelRotation2);
	object3d2->SetScale(modelScale2);
	
	//スプライトの更新
	for (Sprite* sprite : sprites) {
		if (sprite) {
			
			sprite->Update();
		}
	}


	skyBox->SetCamera(camera.get());
	skyBox->Update();

	sceneChangeAnimation->Update(1.0f / 60.0f);

}

void DebugScene::Finalize() {

	for (Sprite* sprite : sprites) {
		if (sprite) {
			delete sprite; // メモリを解放
		}
	}
	sprites.clear(); // ポインタをクリア

}

void DebugScene::Object3DDraw() {
	object3d->Draw();
	object3d2->Draw();

	skyBox->Draw();
}

void DebugScene::SpriteDraw() {
	for (Sprite* sprite : sprites) {
		if (sprite) {
			sprite->Draw();
		}
	}
	
	if (showSceneChangeAnimation) {
		sceneChangeAnimation->Draw();
		if (sceneChangeAnimation->IsFinished()) {
			showSceneChangeAnimation = false;
		}
	}
	
}

void DebugScene::ImGuiDraw() {
	ImGui::Begin("DebugScene");
	ImGui::Text("Hello, DebugScene!");

	// シーンチェンジアニメーション ImGui
	if (ImGui::Button("Start SceneChangeAnimation")) {
		if (sceneChangeAnimation) {
			sceneChangeAnimation->Initialize();
			showSceneChangeAnimation = true;
		}
	}
	ImGui::Checkbox("Show SceneChangeAnimation", &showSceneChangeAnimation);
	if (sceneChangeAnimation) {
		ImGui::Text("Animation Finished: %s", sceneChangeAnimation->IsFinished() ? "Yes" : "No");
	}

	ImGui::End();

	//skyBoxのImGui

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
	
		//スプライトのImGui
		for (Sprite* sprite : sprites) {
			if (sprite) {
				sprite->DrawImGui("sprite");
			}
		}

		object3d->DrawImGui("plane");
		object3d2->DrawImGui("terran");

		static float scratchPosition = 0.0f;
		static bool isScratching = false;
		static float lastScratchPosition = 0.0f;
		//再生時間
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
		//volume
		static float volume = 0.1f;
		ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f);
		audio->SetVolume(volume);
	
		// 再生バー
		static float playbackPosition = 0.0f;
		//再生位置の取得
		playbackPosition = audio->GetPlaybackPosition();
		//再生位置の視認
		ImGui::SliderFloat("Playback Position", &playbackPosition, 0.0f, duration);
		//audio->SetPlaybackPosition(playbackPosition);
	
		//speed
		static float speed = 1.0f;
		ImGui::SliderFloat("Speed", &speed, 0.0f, 2.0f);
		audio->SetPlaybackSpeed(speed);
	
		ImGui::End();
	
	
	#endif // DEBUG

		sceneChangeAnimation->DrawImGui();
}

void DebugScene::ParticleDraw() {

}
