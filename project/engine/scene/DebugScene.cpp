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

	TextureManager::GetInstance()->LoadTexture(testDDSTextureHandle);

	//モデルファイルパス
	const std::string modelFileNamePath = "plane.gltf";
	//モデルファイルパス2
	const std::string modelFileNamePath2 = "terrain.obj";


	//オーディオ
	const std::string audioFileName = "fanfare.wav";
	const std::string audioDirectoryPath = "Resources/Audio/";


#pragma region Audioの初期化
	audio = std::make_unique<Audio>();
	audio->Initialize(audioFileName, audioDirectoryPath);
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

		//各種機能を使えるようにする
		//左右反転
		isFlipX_ = sprite->GetFlipX();
		//上下反転
		isFlipY_ = sprite->GetFlipY();
		//テクスチャの左上座標
		textureLeftTop = sprite->GetTextureLeftTop();
		//テクスチャから初期サイズを得るフラグ
		isAdjustTextureSize = sprite->GetIsAdjustTextureSize();

		//スプライトの位置や回転を設定
		sprite->SetPosition(spritePosition);
		sprite->SetRotation(spriteRotation);
		sprite->SetColor(spriteColor);
		sprite->SetSize(size);
		sprite->SetTextureLeftTop(textureLeftTop);
		sprite->SetGetIsAdjustTextureSize(isAdjustTextureSize);

		sprites.push_back(sprite);

	}
#pragma endregion スプライトの初期化

	/*---------------
	  オブジェクト3D
	---------------*/
#pragma region 3Dモデルの初期化
	//オブジェクト3D

	object3d = std::make_unique<Object3d>();
	object3d->Initialize(modelFileNamePath);
	object3d->SetModel(modelFileNamePath);

	////////////////////////////////////////////////////////////////////////



	//オブジェクト3D

	object3d2 = std::make_unique<Object3d>();
	object3d2->Initialize(modelFileNamePath2);

	object3d2->SetModel(modelFileNamePath2);

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


	skyBox = std::make_unique<SkyBox>();
	skyBox->Initialize(testDDSTextureHandle);
}

void DebugScene::Update() {
	/*--------------
	   ゲームの処理
	--------------*/
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);
	camera->Update();



	//オブジェクト3Dの更新
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
			// ここでは各スプライトの位置や回転を更新する処理を行う
			// 例: X軸方向に少しずつ移動させる
			Vector2 currentPosition = sprite->GetPosition();
			/*currentPosition.x = 100.0f;
			currentPosition.y = 100.0f;*/
			float currentRotation = sprite->GetRotation();

			sprite->SetPosition(currentPosition);
			sprite->SetRotation(currentRotation);
			sprite->SetTextureLeftTop(textureLeftTop);
			sprite->SetFlipX(isFlipX_);
			sprite->SetFlipY(isFlipY_);
			sprite->SetGetIsAdjustTextureSize(isAdjustTextureSize);

			sprite->Update();
		}
	}


	skyBox->SetCamera(camera.get());
	skyBox->Update();



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
	
	object3d2->Draw();

	skyBox->Draw();
}

void DebugScene::SpriteDraw() {
	for (Sprite* sprite : sprites) {
		if (sprite) {
			sprite->Draw();
		}
	}
	object3d->Draw();
}

void DebugScene::ImGuiDraw() {
	ImGui::Begin("DebugScene");
	ImGui::Text("Hello, DebugScene!");
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
		ImGui::End();
	
		//スプライトのImGui
		for (Sprite* sprite : sprites) {
			if (sprite) {
				ImGui::Begin("Sprite");
	
	
				Vector2 spritePosition = sprite->GetPosition();
				ImGui::SliderFloat2("Position", &spritePosition.x, 0.0f, 1920.0f, "%.1f");
				sprite->SetPosition(spritePosition);
	
				ImGui::Checkbox("isFlipX", &isFlipX_);
				ImGui::Checkbox("isFlipY", &isFlipY_);
				ImGui::Checkbox("isAdjustTextureSize", &isAdjustTextureSize);
				ImGui::DragFloat2("textureLeftTop", &textureLeftTop.x);
	
				Vector4 color = sprite->GetColor();
				ImGui::ColorEdit4("Color", &color.x);
				sprite->SetColor(color);
	
				ImGui::End();
			}
		}
		ImGui::Begin("Object3D");
		ImGui::DragFloat3("Position", &modelPosition.x);
		ImGui::DragFloat3("Rotation", &modelRotation.x);
		ImGui::DragFloat3("Scale", &modelScale.x);
	
	
		//色
		Vector4 color = object3d->GetModelColor();
		ImGui::ColorEdit4("Color", &color.x);
		object3d->SetModelColor(color);
	
		ImGui::End();
	
		object3d->ShowImGuiLight();
		ImGui::Begin("Object3D2");
		ImGui::DragFloat3("Position", &modelPosition2.x);
		ImGui::DragFloat3("Rotation", &modelRotation2.x);
		ImGui::DragFloat3("Scale", &modelScale2.x);
	
	
		object3d2->SetModelColor(color);
		ImGui::End();
	
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

}

void DebugScene::ParticleDraw() {

}
