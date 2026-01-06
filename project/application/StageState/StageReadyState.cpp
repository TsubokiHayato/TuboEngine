#include "StageReadyState.h"
#include "LineManager.h"
#include "StageScene.h"
#include "StageType.h"

#undef max
#include <algorithm>

// ユーティリティ: チェビシェフ距離を計算
static int ChebyshevDistance(int x1, int y1, int x2, int y2) { return std::max(std::abs(x1 - x2), std::abs(y1 - y2)); }

// 汎用: マップチップを走査してコールバック
template<typename Func> void ForEachMapChip(StageScene* scene, Func func) {
	auto* field = scene->GetMapChipField();
	for (uint32_t y = 0; y < field->GetNumBlockVirtical(); ++y) {
		for (uint32_t x = 0; x < field->GetNumBlockHorizontal(); ++x) {
			func(x, y, field->GetMapChipTypeByIndex(x, y));
		}
	}
}

void StageReadyState::Enter(StageScene* scene) {
	// 既存スプライトを破棄
	readySprite_.reset();
	startSprite_.reset();

	std::string testDDSTextureHandle = "rostock_laage_airport_4k.dds";
	TextureManager::GetInstance()->LoadTexture(testDDSTextureHandle);
	// マップチップフィールド初期化
	scene->GetMapChipField()->LoadMapChipCsv(scene->GetMapChipCsvFilePath());

	// 先にプレイヤーのマップチップ座標を取得してワールド座標に変換
	int playerMapX = -1, playerMapY = -1;
	ForEachMapChip(scene, [&](uint32_t x, uint32_t y, MapChipType type) {
		if (type == MapChipType::Player) {
			playerMapX = static_cast<int>(x);
			playerMapY = static_cast<int>(y);
		}
	});
	if (playerMapX < 0 || playerMapY < 0) {
		// 見つからない場合は(0,0)にフォールバック
		playerMapX = 0;
		playerMapY = 0;
	}
	playerTargetPosition_ = scene->GetMapChipField()->GetMapChipPositionByIndex(playerMapX, playerMapY);

	// プレイヤー初期化（マップ座標へ配置）
	scene->GetPlayer()->Initialize();
	scene->GetPlayer()->SetMapChipField(scene->GetMapChipField());
	scene->GetPlayer()->SetIsDontMove(true); // 落下中は移動禁止
	scene->GetPlayer()->SetPosition(playerTargetPosition_); // 初期位置をマップチップ座標に設定

	// フォローカメラ初期化（プレイヤー追従）
	scene->GetFollowCamera()->Initialize(scene->GetPlayer(), Vector3 {0.0f, 0.0f, -70.0f}, 0.25f);
	scene->GetFollowCamera()->Update();

	LineManager::GetInstance()->SetDefaultCamera(scene->GetFollowCamera()->GetCamera());

	// ブロック・エネミー・タイルの初期化
	auto& blocks = scene->GetBlocks();
	auto& enemies = scene->GetEnemies();
	auto& tiles = scene->GetTiles();
	blocks.clear();
	enemies.clear();
	tiles.clear();
	blockTargetPositions_.clear();
	blockRippleLayers_.clear();
	enemyTargetPositions_.clear();
	enemyRippleLayers_.clear();
	tileTargetPositions_.clear();
	tileRippleLayers_.clear();

	// マップチップを走査して各オブジェクト生成
	ForEachMapChip(scene, [&](uint32_t x, uint32_t y, MapChipType type) {
		Vector3 pos = scene->GetMapChipField()->GetMapChipPositionByIndex(x, y);
		int layer = ChebyshevDistance(static_cast<int>(x), static_cast<int>(y), playerMapX, playerMapY);

		if (type == MapChipType::kBlock) {
			// ブロック生成
			auto block = std::make_unique<Block>();
			block->Initialize(pos);
			block->SetCamera(scene->GetFollowCamera()->GetCamera());
			block->Update();
			scene->GetBlocks().push_back(std::move(block));
			blockTargetPositions_.push_back(pos);
			blockRippleLayers_.push_back((float)layer);
		} else if (type == MapChipType::Enemy) {
			// エネミー生成
			auto enemy = std::make_unique<RushEnemy>();
			enemy->Initialize();

			enemy->SetCamera(scene->GetFollowCamera()->GetCamera());

			enemy->SetPlayer(scene->GetPlayer());
			enemy->SetPosition(pos);
			enemy->Update();
			scene->GetEnemies().push_back(std::move(enemy));
			enemyTargetPositions_.push_back(pos);
			enemyRippleLayers_.push_back((float)layer);
		}
		// タイルは全マス生成
		auto tile = std::make_unique<Tile>();
		Vector3 tilePos = pos;
		tilePos.z = -1.0f; // ブロックの下に配置
		tile->Initialize(tilePos, {1.0f, 1.0f, 1.0f}, "tile.obj");

		tile->SetCamera(scene->GetFollowCamera()->GetCamera());

		tile->Update();
		scene->GetTiles().push_back(std::move(tile));
		tileTargetPositions_.push_back(tilePos);
		tileRippleLayers_.push_back((float)layer);
	});

	scene->GetPlayer()->SetCamera(scene->GetFollowCamera()->GetCamera());
	scene->GetPlayer()->Update();

	// アニメーション用タイマー初期化
	currentDroppingLayer_ = 0;
	layerDropTimer_ = 0.0f;
	isDropFinished_ = false;
	prevTime_ = std::chrono::steady_clock::now();
	dropDuration_ = 0.2f;

	// READY/START!!スプライト初期化
	readyPhase_ = ReadyStatePhase::Ready;
	readyTimer_ = 0.0f;
	restartWaitTimer_ = 0.0f;

	TextureManager::GetInstance()->LoadTexture("ready.png");
	TextureManager::GetInstance()->LoadTexture("start.png");
	readySprite_ = std::make_unique<Sprite>();
	readySprite_->Initialize("ready.png");
	readySprite_->SetPosition({640.0f, 360.0f});
	readySprite_->SetAnchorPoint({0.5f, 0.5f});

	startSprite_ = std::make_unique<Sprite>();
	startSprite_->Initialize("start.png");
	startSprite_->SetPosition({640.0f, 360.0f});
	startSprite_->SetAnchorPoint({0.5f, 0.5f});

	readySprite_->Update();
	startSprite_->Update();

	scene->GetSkyDome()->Initialize();
	scene->GetSkyDome()->SetCamera(scene->GetFollowCamera()->GetCamera());
	scene->GetSkyDome()->Update();

}

void StageReadyState::Update(StageScene* scene) {
	using namespace std::chrono;
	auto now = steady_clock::now();
	float deltaTime = duration_cast<duration<float>>(now - prevTime_).count();
	prevTime_ = now;

	// フォローカメラ更新
	if (scene->GetFollowCamera()) {
		scene->GetFollowCamera()->Update();
		LineManager::GetInstance()->SetDefaultCamera(scene->GetFollowCamera()->GetCamera());
	}

	// --- リスタート画面表示中 ---
	if (readyPhase_ == ReadyStatePhase::None) {
		restartWaitTimer_ += deltaTime;


		scene->GetFollowCamera()->Update();
		// 通常のオブジェクト更新
		for (auto& block : scene->GetBlocks()) {

			block->SetCamera(scene->GetFollowCamera()->GetCamera());
			block->Update();
		}
		scene->GetPlayer()->SetCamera(scene->GetFollowCamera()->GetCamera());
		scene->GetPlayer()->Update();
		for (auto& enemy : scene->GetEnemies()) {
			enemy->SetCamera(scene->GetFollowCamera()->GetCamera());
			enemy->Update();
		}
		for (auto& tile : scene->GetTiles()) {
			tile->SetCamera(scene->GetFollowCamera()->GetCamera());
			tile->Update();
		}
		return;
	} else {
		restartWaitTimer_ = 0.0f;
	}


	// 常にフォローカメラをデフォルトに
	if (scene->GetFollowCamera()) {
		LineManager::GetInstance()->SetDefaultCamera(scene->GetFollowCamera()->GetCamera());
	}

	// 落下アニメーション中はREADY表示
	if (readyPhase_ == ReadyStatePhase::Ready) {
		readyAppearAnim_ += deltaTime * 2.0f; // 0.5秒で1.0f
		if (readyAppearAnim_ > 1.0f)
			readyAppearAnim_ = 1.0f;

		// S字イージング
		float ease = readyAppearAnim_ < 0.5f ? 2.0f * readyAppearAnim_ * readyAppearAnim_ : 1.0f - powf(-2.0f * readyAppearAnim_ + 2.0f, 2.0f) / 2.0f;

		// スライドイン（y: 600→360）
		float y = 600.0f + (360.0f - 600.0f) * ease;

		// フェードイン
		float alpha = ease;

		readySprite_->SetPosition({640.0f, y});
		readySprite_->SetColor({1, 1, 1, alpha});

	} else {
		readyAppearAnim_ = 0.0f;
	}

	// --- START!!アニメーション（フェードアウト） ---
	if (readyPhase_ == ReadyStatePhase::Start) {
		startAppearAnim_ += deltaTime * 1.5f; // 0.67秒で1.0f
		if (startAppearAnim_ > 1.0f)
			startAppearAnim_ = 1.0f;

		// S字イージング
		float ease = startAppearAnim_ < 0.5f ? 2.0f * startAppearAnim_ * startAppearAnim_ : 1.0f - powf(-2.0f * startAppearAnim_ + 2.0f, 2.0f) / 2.0f;

		// フェードアウト
		float alpha = 1.0f - ease;



		startSprite_->SetColor({1, 1, 1, alpha});

		if (ease >= 1.0f) {
			// STARTアニメーション終了でPlayingへ遷移
			scene->GetStageStateManager()->ChangeState(StageType::Playing, scene);
		}
	} else {
		startAppearAnim_ = 0.0f;
	}

	if (!isDropFinished_) {
		layerDropTimer_ += deltaTime;

		auto lerp = [](float a, float b, float t) { return a + (b - a) * t; };

		// レイヤーごとに落下時間を短縮
		float layerDropDuration = dropDuration_ * static_cast<float>(std::pow(0.8f, currentDroppingLayer_));

		// 各オブジェクトの落下アニメーション
		auto animateDrop = [&](auto& objects, const std::vector<Vector3>& targets, const std::vector<float>& layers) {
			for (size_t i = 0; i < objects.size(); ++i) {
				int layer = (int)layers[i];
				if (layer < currentDroppingLayer_) {
					objects[i]->SetPosition(targets[i]);
				} else if (layer == currentDroppingLayer_) {
					float t = std::min(layerDropTimer_ / layerDropDuration, 1.0f);
					const Vector3& target = targets[i];
					Vector3 start = target;
					start.z += dropOffsetZ_;
					Vector3 pos;
					pos.x = lerp(start.x, target.x, t);
					pos.y = lerp(start.y, target.y, t);
					pos.z = lerp(start.z, target.z, t);
					objects[i]->SetPosition(pos);
				} else {
					Vector3 pos = targets[i];
					pos.z += dropOffsetZ_;
					objects[i]->SetPosition(pos);
				}
			}
		};

		// ブロック・タイル・エネミーの落下
		animateDrop(scene->GetBlocks(), blockTargetPositions_, blockRippleLayers_);
		animateDrop(scene->GetTiles(), tileTargetPositions_, tileRippleLayers_);
		animateDrop(scene->GetEnemies(), enemyTargetPositions_, enemyRippleLayers_);

		// タイルは毎フレームUpdate
		for (auto& tile : scene->GetTiles())
			tile->Update();

		// プレイヤーの落下
		if (currentDroppingLayer_ > 0) {
			scene->GetPlayer()->SetPosition(playerTargetPosition_);
		} else {
			float t = std::min(layerDropTimer_ / layerDropDuration, 1.0f);
			Vector3 start = playerTargetPosition_;
			start.z += dropOffsetZ_;
			Vector3 pos;
			pos.x = lerp(start.x, playerTargetPosition_.x, t);
			pos.y = lerp(start.y, playerTargetPosition_.y, t);
			pos.z = lerp(start.z, playerTargetPosition_.z, t);
			scene->GetPlayer()->SetPosition(pos);
		}

		// レイヤー内の全オブジェクトが落下完了したか判定
		auto isLayerFinished = [&](const std::vector<float>& layers) {
			for (auto l : layers)
				if ((int)l == currentDroppingLayer_ && layerDropTimer_ < layerDropDuration)
					return false;
			return true;
		};
		bool layerFinished = isLayerFinished(blockRippleLayers_) && isLayerFinished(enemyRippleLayers_) && isLayerFinished(tileRippleLayers_);
		if (currentDroppingLayer_ == 0 && layerDropTimer_ < layerDropDuration)
			layerFinished = false;

		// レイヤー進行
		if (layerFinished) {
			++currentDroppingLayer_;
			layerDropTimer_ = 0.0f;
			int maxLayer = 0;
			for (auto l : blockRippleLayers_)
				maxLayer = std::max(maxLayer, (int)l);
			for (auto l : enemyRippleLayers_)
				maxLayer = std::max(maxLayer, (int)l);
			for (auto l : tileRippleLayers_)
				maxLayer = std::max(maxLayer, (int)l);
			if (currentDroppingLayer_ > maxLayer)
				isDropFinished_ = true;
		}

		// 各オブジェクトのUpdate
		scene->GetFollowCamera()->Update();
		for (auto& tile : scene->GetTiles()) {
			tile->SetCamera(scene->GetFollowCamera()->GetCamera());
			tile->Update();
		}
		for (auto& block : scene->GetBlocks()) {
			block->SetCamera(scene->GetFollowCamera()->GetCamera());
			block->Update();
		}
		scene->GetPlayer()->SetCamera(scene->GetFollowCamera()->GetCamera());
		scene->GetPlayer()->Update();
		for (auto& enemy : scene->GetEnemies()) {
			enemy->SetCamera(scene->GetFollowCamera()->GetCamera());
			enemy->Update();
		}

		return;
	} else {
		// 落下アニメーションが終わった瞬間にSTART!!へ
		readyPhase_ = ReadyStatePhase::Start;
		readyTimer_ = 0.0f;
	}

	readySprite_->Update();
	startSprite_->Update();

	for (auto& tile : scene->GetTiles()) {
		tile->SetCamera(scene->GetFollowCamera()->GetCamera());
		tile->Update();
	}

	// 通常のオブジェクト更新
	for (auto& block : scene->GetBlocks()) {
		block->SetCamera(scene->GetFollowCamera()->GetCamera());
		block->Update();
	}
	scene->GetPlayer()->SetCamera(scene->GetFollowCamera()->GetCamera());
	scene->GetPlayer()->Update();
	for (auto& enemy : scene->GetEnemies()) {
		enemy->SetCamera(scene->GetFollowCamera()->GetCamera());
		enemy->Update();
	}
	for (auto& tile : scene->GetTiles()) {
		tile->SetCamera(scene->GetFollowCamera()->GetCamera());
		tile->Update();
	}
	scene->GetSkyDome()->SetCamera(scene->GetFollowCamera()->GetCamera());
	scene->GetSkyDome()->Update();

	LineManager::GetInstance()->SetDefaultCamera(scene->GetFollowCamera()->GetCamera());
}

void StageReadyState::Exit(StageScene* scene) {}

void StageReadyState::Object3DDraw(StageScene* scene) {
	// 3Dオブジェクト描画
	for (auto& block : scene->GetBlocks())
		block->Draw();
	scene->GetPlayer()->Draw();
	for (auto& enemy : scene->GetEnemies())
		enemy->Draw();
	for (auto& tile : scene->GetTiles())
		tile->Draw();

	scene->GetSkyDome()->Draw();
}

void StageReadyState::SpriteDraw(StageScene* scene) {
	if (readyPhase_ == ReadyStatePhase::Ready && readySprite_) {
		readySprite_->Draw();
	} else if (readyPhase_ == ReadyStatePhase::Start && startSprite_) {
		startSprite_->Draw();
	}
}

void StageReadyState::ImGuiDraw(StageScene* scene) {

#ifdef USE_IMGUI
	// カメラ操作用UI（FollowCamera）
	if (scene->GetFollowCamera()) {
		if (ImGui::CollapsingHeader("FollowCamera")) {
			scene->GetFollowCamera()->DrawImGui();
		}
	}

	

	// 落下アニメーション制御UI
	if (ImGui::CollapsingHeader("Drop Animation")) {
		auto easeOutQuad = [](float t) { return 1.0f - (1.0f - t) * (1.0f - t); };
		float t = std::min(layerDropTimer_ / dropDuration_, 1.0f);
		float easedT = easeOutQuad(t);

		ImGui::ProgressBar(t, ImVec2(0.0f, 0.0f), "Progress");
		ImGui::SliderFloat("Drop Duration", &dropDuration_, 0.05f, 2.0f, "%.2f sec");
		ImGui::SliderFloat("Drop Offset Z", &dropOffsetZ_, 1.0f, 100.0f, "%.1f");

		ImGui::SameLine();
		if (ImGui::Button("Skip Animation")) {
			int maxLayer = 0;
			for (auto l : blockRippleLayers_)
				maxLayer = std::max(maxLayer, (int)l);
			for (auto l : enemyRippleLayers_)
				maxLayer = std::max(maxLayer, (int)l);
			for (auto l : tileRippleLayers_)
				maxLayer = std::max(maxLayer, (int)l);
			currentDroppingLayer_ = maxLayer + 1;
			isDropFinished_ = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Restart Animation")) {
			Enter(scene);
		}

		ImGui::Text("isDropFinished: %s", isDropFinished_ ? "true" : "false");

		//現在のPhase状態
		const char* phaseStr = "";
		switch (readyPhase_) {
		case ReadyStatePhase::Ready:
			phaseStr = "Ready";
			break;
		case ReadyStatePhase::Start:
			phaseStr = "Start";
			break;
		case ReadyStatePhase::None:
			phaseStr = "None";
			break;
		}
		ImGui::Text("Current Phase: %s", phaseStr);


	}
#endif // USE_IMGUI
}

void StageReadyState::ParticleDraw(StageScene* scene) {}
