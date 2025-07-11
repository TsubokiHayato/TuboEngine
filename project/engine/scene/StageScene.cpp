#include "StageScene.h"

#include "CollisionManager.h"
#include "StageScene.h"
#include "application/FollowTopDownCamera.h"

void StageScene::Initialize(Object3dCommon* object3dCommon, SpriteCommon* spriteCommon, ParticleCommon* particleCommon, WinApp* winApp, DirectXCommon* dxCommon) {
	this->object3dCommon = object3dCommon;
	this->spriteCommon = spriteCommon;
	this->winApp = winApp;
	this->dxCommon = dxCommon;

	// �v���C���[
	player_ = std::make_unique<Player>();
	player_->Initialize(object3dCommon);

	// �Ǐ]�J�����̐����E������
	followCamera = std::make_unique<FollowTopDownCamera>();
	followCamera->Initialize(player_.get(), Vector3(0.0f, 40.0f, 0.0f), 0.2f);

	// �v���C���[�ɃJ�������Z�b�g
	player_->SetCamera(followCamera->GetCamera());

	// Enemy���X�g
	enemies.clear();
	const int enemyCount = 1;
	for (int i = 0; i < enemyCount; ++i) {
		auto enemy = std::make_unique<Enemy>();
		enemy->SetParticleCommon(particleCommon);
		enemy->Initialize(object3dCommon);
		enemy->SetCamera(followCamera->GetCamera());
		enemy->SetPosition(Vector3(float(i * 2), 0.0f, 5.0f));
		enemies.push_back(std::move(enemy));
	}

	// �Փ˃}�l�[�W���̐���
	collisionManager_ = std::make_unique<CollisionManager>();
	collisionManager_->Initialize();

}

void StageScene::Update()
{
}

	player_->SetCamera(followCamera->GetCamera());
	player_->Update();

	
	for (auto& enemy : enemies) {
		enemy->SetCamera(followCamera->GetCamera());
		enemy->Update();
	}

	collisionManager_->Update();
	CheckAllCollisions();
}



void StageScene::Finalize()
{
}

void StageScene::Object3DDraw() {
	// 3D�I�u�W�F�N�g�̕`��
	// �v���C���[��3D�I�u�W�F�N�g��`��
	player_->Draw();

	// �G��3D�I�u�W�F�N�g��`��
	for (auto& enemy : enemies) {
		enemy->Draw();
	}
	// �����蔻��̉���
	collisionManager_->Draw();
}
void StageScene::SpriteDraw()
{
}

void StageScene::ImGuiDraw()
{
	// Camera��ImGui
	followCamera->DrawImGui();

	// Player��ImGui
	player_->DrawImGui();


	//Enemy��Imgui
	for (auto& enemy : enemies) {
		enemy->DrawImGui();
	}


}

void StageScene::ParticleDraw() {
	for (auto& enemy : enemies) {
		enemy->ParticleDraw();
	}
	}

void StageScene::CheckAllCollisions() {
	/// �Փ˃}�l�[�W���̃��Z�b�g ///
	collisionManager_->Reset();

	/// �R���C�_�[�����X�g�ɓo�^ ///
	collisionManager_->AddCollider(player_.get());

	// �����̓G��o�^ ///
	for (const auto& enemy : enemies) {
		collisionManager_->AddCollider(enemy.get());
	}

	  // �v���C���[�̒e
	for (const auto& bullet : player_->GetBullets()) {
		collisionManager_->AddCollider(bullet.get());
	}



	// �v���C���[�����S������R���C�_�[���폜�܂��͓G�����S������R���C�_�[���폜 ///
	//if (player_->GetHP() <= 0 || enemy_->GetHP() <= 0) {
	//	collisionManager_->RemoveCollider(player_.get());
	//	collisionManager_->RemoveCollider(enemy_.get());

	//}

	// �Փ˔���Ɖ���
	collisionManager_->CheckAllCollisions();


}
