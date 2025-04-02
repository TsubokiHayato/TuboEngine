#include "DebugScene.h"
#include"ImGuiManager.h"
#include"SceneManager.h"
#include"ModelManager.h"
#include"TextureManager.h"
#include"BlendMode.h"
void DebugScene::Initialize(Object3dCommon* object3dCommon, SpriteCommon* spriteCommon, ParticleCommon* particleCommon, WinApp* winApp, DirectXCommon* dxCommon) {

	this->object3dCommon = object3dCommon;
	this->spriteCommon = spriteCommon;
	this->winApp = winApp;
	this->dxCommon = dxCommon;
	//テクスチャマネージャに追加する画像ハンドル
	std::string uvCheckerTextureHandle = "uvChecker.png";
	std::string monsterBallTextureHandle = "monsterBall.png";

	//画像ハンドルをテクスチャマネージャに挿入する
	TextureManager::GetInstance()->LoadTexture(uvCheckerTextureHandle);
	TextureManager::GetInstance()->LoadTexture(monsterBallTextureHandle);

	//モデルディレクトリパス
	const std::string modelDirectoryPath = "Resources";
	//モデルファイルパス
	const std::string modelFileNamePath = "sphere.obj";
	//モデルファイルパス2
	const std::string modelFileNamePath2 = "terrain.obj";

	ModelManager::GetInstance()->LoadModel(modelFileNamePath);
	ModelManager::GetInstance()->LoadModel(modelFileNamePath2);

	const std::string audioFileName = "fanfare.wav";
	const std::string audioDirectoryPath = "Resources/Audio/";


#pragma region Audioの初期化
	audio = std::make_unique<Audio>();
	audio->Initialize(audioFileName, audioDirectoryPath);
	audio->Play(true);

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
			sprite->Initialize(this->spriteCommon, monsterBallTextureHandle);
		} else {
			//uvCheckerを表示させる
			sprite->Initialize(this->spriteCommon, uvCheckerTextureHandle);
		}


		// 各スプライトに異なる位置やプロパティを設定する
		//Vector2 spritePosition = { i * -1280.0f, 0.0f }; // スプライトごとに異なる位置
		Vector2 spritePosition = { 100.0f, 100.0f }; // スプライトごとに異なる位置
		float spriteRotation = 0.0f;                 // 回転は任意
		Vector4 spriteColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // 色は白（RGBA）
		Vector2 size = { 50.0f, 50.0f };             // 任意のサイズ

		//各種機能を使えるようにする
		isFlipX_ = sprite->GetFlipX();
		isFlipY_ = sprite->GetFlipY();
		textureLeftTop = sprite->GetTextureLeftTop();
		isAdjustTextureSize = sprite->GetIsAdjustTextureSize();


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
	object3d->Initialize(this->object3dCommon);
	object3d->SetModel(modelFileNamePath);

	////////////////////////////////////////////////////////////////////////



	//オブジェクト3D

	object3d2 = std::make_unique<Object3d>();
	object3d2->Initialize(this->object3dCommon);

	object3d2->SetModel(modelFileNamePath2);

#pragma endregion 3Dモデルの初期化

#pragma region cameraの初期化
	//カメラ

	camera = std::make_unique<Camera>();

	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);
	object3dCommon->SetDefaultCamera(camera.get());
	object3d->SetCamera(camera.get());
	object3d2->SetCamera(camera.get());

#pragma endregion cameraの初期化
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
}

void DebugScene::SpriteDraw() {
	for (Sprite* sprite : sprites) {
		if (sprite) {
			sprite->Draw();
		}
	}
}

void DebugScene::ImGuiDraw() {
	ImGui::Begin("DebugScene");
	ImGui::Text("Hello, DebugScene!");
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

	ImGui::Begin("Light");
	//光源のタイプ
	lightType = object3d->GetLightType();
	ImGui::SliderInt("LightType", &lightType, 0, 4);
	object3d->SetLightType(lightType);
	object3d2->SetLightType(lightType);


	//光沢度
	shininess = object3d->GetLightShininess();
	ImGui::DragFloat("Shininess", &shininess);
	object3d->SetLightShininess(shininess);
	object3d2->SetLightShininess(shininess);

	//光源の色
	lightColor = object3d->GetLightColor();
	ImGui::ColorEdit4("LightColor", &lightColor.x);
	object3d->SetLightColor(lightColor);
	object3d2->SetLightColor(lightColor);

	//光源の方向
	lightDirection = object3d->GetLightDirection();
	ImGui::DragFloat3("LightDirection", &lightDirection.x, 0.1f);
	object3d->SetLightDirection(lightDirection);
	object3d2->SetLightDirection(lightDirection);

	//光源の強さ
	intensity = object3d->GetLightIntensity();
	ImGui::DragFloat("LightIntensity", &intensity, 0.1f);
	object3d->SetLightIntensity(intensity);
	object3d2->SetLightIntensity(intensity);




	ImGui::Text("PointLight");
	//光源の色
	pointLightColor = object3d->GetPointLightColor();
	ImGui::ColorEdit4("PointLightColor", &pointLightColor.x);
	object3d->SetPointLightColor(pointLightColor);
	object3d2->SetPointLightColor(pointLightColor);

	//光源の位置
	pointLightPosition = object3d->GetPointLightPosition();
	ImGui::DragFloat3("PointLightPosition", &pointLightPosition.x, 0.1f);
	object3d->SetPointLightPosition(pointLightPosition);
	object3d2->SetPointLightPosition(pointLightPosition);

	//光源の強さ
	pointLightIntensity = object3d->GetPointLightIntensity();
	ImGui::DragFloat("PointLightIntensity", &pointLightIntensity, 0.1f);
	object3d->SetPointLightIntensity(pointLightIntensity);
	object3d2->SetPointLightIntensity(pointLightIntensity);
	ImGui::End();

	ImGui::Begin("SpotLight");
	//光源の色
	Vector4 spotLightColor;
	object3d->GetSpotLightColor(spotLightColor);
	ImGui::ColorEdit4("SpotLightColor", &spotLightColor.x);
	object3d->SetSpotLightColor(spotLightColor);
	object3d2->SetSpotLightColor(spotLightColor);

	//光源の位置
	Vector3 spotLightPosition;
	object3d->GetSpotLightPosition(spotLightPosition);
	ImGui::DragFloat3("SpotLightPosition", &spotLightPosition.x, 0.1f);
	object3d->SetSpotLightPosition(spotLightPosition);
	object3d2->SetSpotLightPosition(spotLightPosition);

	//光源の方向
	Vector3 spotLightDirection;
	object3d->GetSpotLightDirection(spotLightDirection);
	ImGui::DragFloat3("SpotLightDirection", &spotLightDirection.x, 0.1f);
	object3d->SetSpotLightDirection(spotLightDirection);
	object3d2->SetSpotLightDirection(spotLightDirection);

	//光源の強さ
	float spotLightIntensity = object3d->GetSpotLightIntensity();
	ImGui::DragFloat("SpotLightIntensity", &spotLightIntensity, 0.1f);
	object3d->SetSpotLightIntensity(spotLightIntensity);
	object3d2->SetSpotLightIntensity(spotLightIntensity);
		
	//光源の距離
	float spotLightDistance = object3d->GetSpotLightDistance();
	ImGui::DragFloat("SpotLightDistance", &spotLightDistance, 0.1f);
	object3d->SetSpotLightDistance(spotLightDistance);
	object3d2->SetSpotLightDistance(spotLightDistance);

	//光源の減衰
	float spotLightDecay = object3d->GetSpotLightDecay();
	ImGui::DragFloat("SpotLightDecay", &spotLightDecay, 0.1f);
	object3d->SetSpotLightDecay(spotLightDecay);
	object3d2->SetSpotLightDecay(spotLightDecay);

	//光源の角度
	float spotLightCosAngle = object3d->GetSpotLightCosAngle();
	ImGui::DragFloat("SpotLightCosAngle", &spotLightCosAngle, 0.1f);
	object3d->SetSpotLightCosAngle(spotLightCosAngle);
	object3d2->SetSpotLightCosAngle(spotLightCosAngle);

	ImGui::End();


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
