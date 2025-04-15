#include "TitleScene.h"
#include"TextureManager.h"
#include"ImGuiManager.h"
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
	std::string particleTextureHandle = "particle.png";

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
		{0.05f, 0.1f, 0.1f},
		//Rotate
		{0.0f, 0.0f, 0.0f},
		//Translate
		{0.0f, 0.0f, 0.0f}

	};
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
			0.0f,
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

	//パーティクル
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
	ImGui::DragFloat3("Position", &cameraPosition.x);
	ImGui::DragFloat3("Rotation", &cameraRotation.x);
	ImGui::DragFloat3("Scale", &cameraScale.x);
	ImGui::End();


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
