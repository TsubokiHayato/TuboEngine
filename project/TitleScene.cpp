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
	particle = new Particle();
	particle->Initialize(this->particleCommon);
	particle->SetScale({ 1.0f,1.0f,1.0f });
	particle->SetRotation({ 0.0f,0.0f,0.0f });
	particle->SetPosition({ 0.0f,0.0f,0.0f });
	particle->SetCamera(camera);
	particle->CreateParticleGroup("Particle",monsterBallTextureHandle);


}

void TitleScene::Update()
{
	particle->Update();

}

void TitleScene::Finalize()
{
	delete camera;
	delete particle;
}

void TitleScene::Object3DDraw()
{
}

void TitleScene::SpriteDraw()
{
}

void TitleScene::ImGuiDraw()
{

	
	
}

void TitleScene::ParticleDraw()
{
	particle->Draw();
}
