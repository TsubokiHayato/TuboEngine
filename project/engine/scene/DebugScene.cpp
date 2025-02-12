#include "DebugScene.h"
#include"ImGuiManager.h"
#include"SceneManager.h"
#include"ModelManager.h"
#include"TextureManager.h"
#include"BlendMode.h"
void DebugScene::Initialize(Object3dCommon* object3dCommon, SpriteCommon* spriteCommon, ParticleCommon* particleCommon, WinApp* winApp, DirectXCommon* dxCommon)
{

	this->object3dCommon = object3dCommon;
	this->spriteCommon = spriteCommon;
	this->winApp = winApp;
	this->dxCommon = dxCommon;
	//テクスチャマネージャに追加する画像ハンドル
	std::string uvCheckerTextureHandle = "Resources/uvChecker.png";
	std::string monsterBallTextureHandle = "Resources/monsterBall.png";

	//画像ハンドルをテクスチャマネージャに挿入する
	TextureManager::GetInstance()->LoadTexture(uvCheckerTextureHandle);
	TextureManager::GetInstance()->LoadTexture(monsterBallTextureHandle);

	//モデルディレクトリパス
	const std::string modelDirectoryPath = "Resources";
	//モデルファイルパス
	const std::string modelFileNamePath = "sphere.obj";
	//モデルファイルパス2
	const std::string modelFileNamePath2 = "barrier.obj";

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
		}
		else {
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

void DebugScene::Update()
{
	/*--------------
	   ゲームの処理
	--------------*/
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);
	camera->Update();

	modelRotation.y += 0.01f;

	modelRotation2.x -= 0.01f;
	//modelRotation2.y -= 0.01f;
	modelRotation2.z -= 0.01f;

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

void DebugScene::Finalize()
{

	for (Sprite* sprite : sprites) {
		if (sprite) {
			delete sprite; // メモリを解放
		}
	}
	sprites.clear(); // ポインタをクリア

}

void DebugScene::Object3DDraw()
{
	object3d->Draw();
	object3d2->Draw();
}

void DebugScene::SpriteDraw()
{
	for (Sprite* sprite : sprites) {
		if (sprite) {
			sprite->Draw();
		}
	}
}

void DebugScene::ImGuiDraw()
{
    ImGui::Begin("DebugScene");
    ImGui::Text("Hello, DebugScene!");
    ImGui::End();

#ifdef _DEBUG

    ImGui::Begin("camera");
    ImGui::DragFloat3("Position", &cameraPosition.x);
    ImGui::DragFloat3("Rotation", &cameraRotation.x);
    ImGui::DragFloat3("Scale", &cameraScale.x);
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

	ImGui::Text("Light");
	//光沢度
	static float shininess = object3d->GetLightShininess();
	ImGui::DragFloat("Shininess", &shininess,0.1f);
	object3d->SetLightShininess(shininess);

	//光源の色
	Vector4 lightColor = object3d->GetLightColor();
	ImGui::ColorEdit4("LightColor", &lightColor.x);
	object3d->SetLightColor(lightColor);

	//光源の方向
	Vector3 lightDirection = object3d->GetLightDirection();
	ImGui::DragFloat3("LightDirection", &lightDirection.x,0.1f);
	object3d->SetLightDirection(lightDirection);

	//光源の強さ
	float lightIntensity = object3d->GetLightIntensity();
	ImGui::DragFloat("LightIntensity", &lightIntensity, 0.1f);
	object3d->SetLightIntensity(lightIntensity);

	//光源のタイプ
	int lightType = object3d->GetLightType();
	ImGui::SliderInt("LightType", &lightType, 0, 2);
	object3d->SetLightType(lightType);


    ImGui::End();

    ImGui::Begin("Object3D2");
    ImGui::DragFloat3("Position", &modelPosition2.x);
    ImGui::DragFloat3("Rotation", &modelRotation2.x);
    ImGui::DragFloat3("Scale", &modelScale2.x);

	Vector4 color2 = object3d2->GetModelColor();
	ImGui::ColorEdit4("Color", &color2.x);
	object3d2->SetModelColor(color2);

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

void DebugScene::ParticleDraw()
{

}
