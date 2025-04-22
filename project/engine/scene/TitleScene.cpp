#include "TitleScene.h"
#include"TextureManager.h"
#include"ImGuiManager.h"
#include"numbers"
void TitleScene::Initialize(Object3dCommon* object3dCommon, SpriteCommon* spriteCommon, ParticleCommon* particleCommon, WinApp* winApp, DirectXCommon* dxCommon) {
	//各共通部分のポインタを受け取る
	this->particleCommon = particleCommon;
	this->dxCommon = dxCommon;
	this->winApp = winApp;
	this->object3dCommon = object3dCommon;
	this->spriteCommon = spriteCommon;

	//カメラ
	camera = std::make_unique<Camera>();
	camera->SetTranslate({ 0.0f,0.0f,-5.0f });
	camera->setRotation({ 0.0f,0.0f,0.0f });
	camera->setScale({ 1.0f,1.0f,1.0f });

	//テクスチャマネージャに追加する画像ハンドル
	std::string uvCheckerTextureHandle = "uvChecker.png";
	std::string monsterBallTextureHandle = "monsterBall.png";
	std::string particleTextureHandle = "gradationLine.png";

	//画像ハンドルをテクスチャマネージャに挿入する
	TextureManager::GetInstance()->LoadTexture(uvCheckerTextureHandle);
	TextureManager::GetInstance()->LoadTexture(monsterBallTextureHandle);
	TextureManager::GetInstance()->LoadTexture(particleTextureHandle);


	

	//パーティクル
	particle = std::make_unique<Particle>();
	particle->Initialize(this->particleCommon);
	particle->CreateParticleGroup("Particle", particleTextureHandle);
	particleTranslate = {
		//Scale
		{1.0f, 1.0f, 1.0f},
		//Rotate
		{0.0f, 0.0f, 0.0f},
		//Translate
		{0.0f, 0.0f, 0.0f}

	};

	
	particleVelocity = {};
	particleColor = { 1.0f,1.0f,1.0f,1.0f };
	particleLifeTime = 100.0f;
	particleCurrentTime= 0.0f;

	particleEmitter_ =
		std::make_unique<ParticleEmitter>(
			//パーティクルのインスタンス
			particle.get(),
			//パーティクルグループ名
			"Particle",
			//エミッターの位置・回転・スケール
			particleTranslate,
			//速度
			particleVelocity,
			// カラー
			particleColor,
			//寿命
			particleLifeTime,
			//経過時間
			particleCurrentTime,
			//発生させるパーティクルの数
			3,
			//発生頻度
			1.0f,
			//繰り返し発生させるかどうかのフラグ
			true
		);

	


}

void TitleScene::Update() {
	//カメラ
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);
	camera->Update();

	
	
	const uint32_t kRingDivide = 32;
	const float kRingRadius = 1.0f;
	const float kRingHeight = 0.2f;
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kRingDivide);
	


	///Particle///
	///パーティクルで乱数を使いたい場合
	/// SetDist～～～の関数を使う
	///基本RangeはScale以外は0.0fなので
	/// 使いたいものの数値を変換して使ってください
	/*particle->SetDistRotateZ(0.0f, 3.14f);
	particle->SetDistScaleY(0.4f, 1.5f);
	*/
	
	particle->SetCamera(camera.get());
	particle->Update();
	particleEmitter_->Update();

}

void TitleScene::Finalize() {


}

void TitleScene::Object3DDraw() {}

void TitleScene::SpriteDraw() {}

void TitleScene::ImGuiDraw() {
	//カメラ
	ImGui::Begin("camera");
	ImGui::DragFloat3("Position", &cameraPosition.x,0.01f);
	ImGui::DragFloat3("Rotation", &cameraRotation.x,0.01f);
	ImGui::DragFloat3("Scale", &cameraScale.x,0.01f);
	ImGui::End();



	Vector3 translate = particle->GetPosition();
	Vector3 scale = particle->GetScale();
	Vector3 rotate = particle->GetRotation();
	//エミッター
	ImGui::Begin("ParticleEmitter");
	ImGui::DragFloat3("Position", &translate.x);
	ImGui::DragFloat3("Rotation", &rotate.x);
	ImGui::DragFloat3("Scale", &scale.x);
	ImGui::End();

	//パーティクル
	ImGui::Begin("Particle");
	ImGui::DragFloat3("Position", &particleTranslate.translate.x);
	ImGui::DragFloat3("Rotation", &particleTranslate.rotate.x);
	ImGui::DragFloat3("Scale", &particleTranslate.scale.x);
	ImGui::End();


}

void TitleScene::ParticleDraw() {
	//パーティクル
	particle->Draw();

}
