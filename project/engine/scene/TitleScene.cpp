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
	camera->SetTranslate({ 0.0f,0.0f,-10.0f });
	camera->setRotation({ 0.0f,0.0f,0.0f });
	camera->setScale({ 1.0f,1.0f,1.0f });

	//テクスチャマネージャに追加する画像ハンドル
	std::string uvCheckerTextureHandle = "uvChecker.png";
	std::string monsterBallTextureHandle = "monsterBall.png";
	std::string particleTextureHandle = "YellowConcentrationLine.png";

	//画像ハンドルをテクスチャマネージャに挿入する
	TextureManager::GetInstance()->LoadTexture(uvCheckerTextureHandle);
	TextureManager::GetInstance()->LoadTexture(monsterBallTextureHandle);
	TextureManager::GetInstance()->LoadTexture(particleTextureHandle);


	


	// Original
	particle = std::make_unique<Particle>();
	particle->Initialize(this->particleCommon, ParticleType::Original);
	particle->CreateParticleGroup("Particle", particleTextureHandle);
	particleTranslate = {
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.3f, 0.0f}
	};

	particleEmitter_ = std::make_unique<ParticleEmitter>(
		particle.get(), "Particle", particleTranslate, particleVelocity, particleColor,
		particleLifeTime, particleCurrentTime, 8, 1.0f, true
	);


	


}

void TitleScene::Update() {
	//カメラ
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);
	camera->Update();

	


	///Particle///
	// Original
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
	particle->Draw();
	
}
