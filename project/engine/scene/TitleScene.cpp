#include "TitleScene.h"
#include"TextureManager.h"
#include"ImGuiManager.h"
void TitleScene::Initialize(Object3dCommon* object3dCommon, SpriteCommon* spriteCommon, ParticleCommon* particleCommon, WinApp* winApp, DirectXCommon* dxCommon)
{
	this->particleCommon = particleCommon;
	this->dxCommon = dxCommon;
	this->winApp = winApp;
	this->object3dCommon = object3dCommon;
	this->spriteCommon = spriteCommon;

	//カメラ
	camera = new Camera();
	camera->SetTranslate({ 0.0f,0.0f,-5.0f });
	camera->setRotation({ 0.0f,0.0f,0.0f });
	camera->setScale({ 1.0f,1.0f,1.0f });

	//テクスチャマネージャに追加する画像ハンドル
	std::string uvCheckerTextureHandle = "Resources/uvChecker.png";
	std::string monsterBallTextureHandle = "Resources/monsterBall.png";

	//画像ハンドルをテクスチャマネージャに挿入する
	TextureManager::GetInstance()->LoadTexture(uvCheckerTextureHandle);
	TextureManager::GetInstance()->LoadTexture(monsterBallTextureHandle);
	
	//パーティクル
	particle =std::make_unique<Particle>();
	particle->Initialize(this->particleCommon);
	particle->CreateParticleGroup("Particle",monsterBallTextureHandle);
	particleEmitter_ = std::make_unique<ParticleEmitter>(
		particle.get(), "Particle",
		Transform{
			{0.2f, 0.2f, 0.2f},
			{0.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 0.0f}
		},
		100, 0.1f, true);


}

void TitleScene::Update()
{
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);
	camera->Update();

	particle->Update();
	particleEmitter_->Update();

}

void TitleScene::Finalize()
{
	delete camera;
	
}

void TitleScene::Object3DDraw()
{
}

void TitleScene::SpriteDraw()
{
}

void TitleScene::ImGuiDraw()
{
	
	ImGui::Begin("camera");
	ImGui::DragFloat3("Position", &cameraPosition.x);
	ImGui::DragFloat3("Rotation", &cameraRotation.x);
	ImGui::DragFloat3("Scale", &cameraScale.x);
	ImGui::End();
	
	
}

void TitleScene::ParticleDraw()
{
	particle->Draw();

}
