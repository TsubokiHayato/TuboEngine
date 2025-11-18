#include "StageScene.h"
#include "Collider/CollisionManager.h"
#include "Camera/FollowTopDownCamera.h"
#include "LineManager.h"

void StageScene::Initialize() {

	///--------------------------------------------------------
	///				メンバ変数の生成
	///--------------------------------------------------------

	mapChipField_ = std::make_unique<MapChipField>();
	player_ = std::make_unique<Player>();
	followCamera = std::make_unique<FollowTopDownCamera>();
	camera = std::make_unique<Camera>();
	collisionManager_ = std::make_unique<CollisionManager>();
	stateManager_ = std::make_unique<StageStateManager>();
	skyDome_ = std::make_unique<SkyDome>();
	sceneChangeAnimation_ = std::make_unique<SceneChangeAnimation>(1280, 720, 80, 1.5f, "barrier.png");
	
	// 衝突マネージャの生成
	collisionManager_->Initialize();
	// ステートマネージャの生成
	stateManager_->Initialize(this);
	// シーンチェンジアニメーション初期化（シーン開始時はDisappearingで覆いを消す）
	sceneChangeAnimation_->Initialize();
	isRequestSceneChange = false;
	
}

void StageScene::Update() {

	sceneChangeAnimation_->Update(1.0f / 60.0f);

	
	if (sceneChangeAnimation_->IsFinished()) {
		// ステートマネージャの更新
		if (stateManager_) {
			stateManager_->Update(this);
		}
	}
	// デフォルトカメラをFollowCameraに設定
	LineManager::GetInstance()->SetDefaultCamera(followCamera->GetCamera());
	// 衝突マネージャの更新
	collisionManager_->Update();
	// 全ての衝突をチェック
	CheckAllCollisions();

}

void StageScene::Finalize() {}

void StageScene::Object3DDraw() {

	// ステートマネージャの3Dオブジェクト描画
	if (stateManager_) {
		stateManager_->Object3DDraw(this);
	}


	// 当たり判定の可視化
	//collisionManager_->Draw();

	// グリッド描画（回転対応）
	//LineManager::GetInstance()->DrawGrid(10000.0f, 1000, {DirectX::XM_PIDIV2, 0.0f, 0.0f});
}

void StageScene::SpriteDraw() {
	
	// ステートマネージャのスプライト描画
	if (stateManager_) {
		stateManager_->SpriteDraw(this);
	}
	
	// アニメーション描画
	if (sceneChangeAnimation_) {
		sceneChangeAnimation_->Draw();
	}
	
	 }

void StageScene::ImGuiDraw() {


// DrawLineのImGui

	#ifdef _DEBUG

	LineManager::GetInstance()->DrawImGui();
	stateManager_->DrawImGui(this);
#endif // _DEBUG

}

void StageScene::ParticleDraw() {
	
	if (stateManager_) {
		stateManager_->ParticleDraw(this);
	}
}
void StageScene::CheckAllCollisions() {
	/// 衝突マネージャのリセット ///
	collisionManager_->Reset();

	/// コライダーをリストに登録 ///
	// プレイヤー
	collisionManager_->AddCollider(player_.get());

	// 敵（生存中のみ登録）
	for (const auto& enemy : enemies) {
		if (!enemy) {
			continue;
		}
		if (enemy->GetIsAllive() && enemy->GetHP() > 0) {
			collisionManager_->AddCollider(enemy.get());
		}
	}

	// プレイヤーの弾（生存中のみ登録）
	for (const auto& bullet : player_->GetBullets()) {
		if (!bullet) {
			continue;
		}
		if (bullet->GetIsAlive()) {
			collisionManager_->AddCollider(bullet.get());
		}
	}

	// 衝突判定と応答
	collisionManager_->CheckAllCollisions();
}