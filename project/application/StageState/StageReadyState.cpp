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

	// カメラ初期化
	if (scene->GetMainCamera()) {
		auto* map = scene->GetMapChipField();
		float centerX = (map->GetNumBlockHorizontal() * MapChipField::GetBlockWidth()) / 8.0f;
		float centerY = (map->GetNumBlockVirtical() * MapChipField::GetBlockHeight()) / 2.0f;
		scene->GetMainCamera()->SetTranslate({9.5f, centerY, 70.0f});
		scene->GetMainCamera()->setRotation({3.14f, 0.0f, 0.0f});
		scene->GetMainCamera()->setScale({1.0f, 1.0f, 1.0f});
		scene->GetMainCamera()->Update();
	}

	// プレイヤーのマップチップ座標取得
	int playerMapX = -1, playerMapY = -1;
	ForEachMapChip(scene, [&](uint32_t x, uint32_t y, MapChipType type) {
		if (type == MapChipType::Player) {
			playerMapX = x;
			playerMapY = y;
		}
	});

	// プレイヤー初期化
	scene->GetPlayer()->Initialize();
	scene->GetPlayer()->SetCamera(scene->GetMainCamera());
	scene->GetPlayer()->SetMapChipField(scene->GetMapChipField());
	scene->GetPlayer()->SetIsDontMove(true); // 落下中は移動禁止
	scene->GetPlayer()->Update();
	playerTargetPosition_ = scene->GetMapChipField()->GetMapChipPositionByIndex(playerMapX, playerMapY);

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
		int layer = ChebyshevDistance(x, y, playerMapX, playerMapY);

		if (type == MapChipType::kBlock) {
			// ブロック生成
			auto block = std::make_unique<Block>();
			block->Initialize(pos);
			block->SetCamera(scene->GetMainCamera());
			block->Update();
			blocks.push_back(std::move(block));
			blockTargetPositions_.push_back(pos);
			blockRippleLayers_.push_back((float)layer);
		} else if (type == MapChipType::Enemy) {
			// エネミー生成
			auto enemy = std::make_unique<Enemy>();
			enemy->Initialize();
			enemy->SetCamera(scene->GetMainCamera());
			enemy->SetPlayer(scene->GetPlayer());
			enemy->SetPosition(pos);
			enemy->Update();
			enemies.push_back(std::move(enemy));
			enemyTargetPositions_.push_back(pos);
			enemyRippleLayers_.push_back((float)layer);
		}
		// タイルは全マス生成
		auto tile = std::make_unique<Tile>();
		Vector3 tilePos = pos;
		tilePos.z = -1.0f; // ブロックの下に配置
		tile->Initialize(tilePos, {1.0f, 1.0f, 1.0f}, "tile.obj");
		tile->SetCamera(scene->GetMainCamera());
		tile->Update();
		tiles.push_back(std::move(tile));
		tileTargetPositions_.push_back(tilePos);
		tileRippleLayers_.push_back((float)layer);
	});

	// アニメーション用タイマー初期化
	currentDroppingLayer_ = 0;
	layerDropTimer_ = 0.0f;
	isDropFinished_ = false;
	prevTime_ = std::chrono::steady_clock::now();
	dropDuration_ = 0.2f;

	// READY/START!!スプライト初期化
	readyPhase_ = ReadyStatePhase::Ready;
	readyTimer_ = 0.0f;

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

	restartSprite_ = std::make_unique<Sprite>();
	restartSprite_->Initialize("restart.png");
	restartSprite_->SetPosition({640.0f, 680.0f});
	restartSprite_->SetAnchorPoint({0.5f, 0.5f});

	readySprite_->Update();
	startSprite_->Update();
	restartSprite_->Update();
}

// 追加: リスタート画面表示用タイマー
static float restartWaitTimer = 0.0f;

void StageReadyState::Update(StageScene* scene) {
	using namespace std::chrono;
	auto now = steady_clock::now();
	float deltaTime = duration_cast<duration<float>>(now - prevTime_).count();
	prevTime_ = now;

	// --- リスタート画面表示中 ---
	if (readyPhase_ == ReadyStatePhase::None) {
		restartWaitTimer_ += deltaTime;

		// restartSprite_のアニメーション（点滅など）
		float alpha = 0.5f + 0.5f * std::sin(restartWaitTimer_ * 3.0f);
		restartSprite_->SetColor({1, 1, 1, alpha});
		restartSprite_->Update();

		// 1.0秒以上経過してからキー入力を受け付ける
		if (restartWaitTimer_ > 1.0f && Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			Enter(scene);
			restartWaitTimer_ = 0.0f;
			return;
		}

		// 通常のオブジェクト更新
		for (auto& block : scene->GetBlocks())
			block->Update();
		scene->GetPlayer()->Update();
		for (auto& enemy : scene->GetEnemies())
			enemy->Update();
		for (auto& tile : scene->GetTiles())
			tile->Update();
		LineManager::GetInstance()->SetDefaultCamera(scene->GetFollowCamera()->GetCamera());
		return;
	} else {
		restartWaitTimer_ = 0.0f;
	}

	// Rキーで全アニメーションをリスタート
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		Enter(scene);
		return;
	}

	LineManager::GetInstance()->SetDefaultCamera(scene->GetMainCamera());

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

	// --- START!!アニメーション（拡大しながらフェードアウト） ---
	if (readyPhase_ == ReadyStatePhase::Start) {
		startAppearAnim_ += deltaTime * 1.5f; // 0.67秒で1.0f
		if (startAppearAnim_ > 1.0f)
			startAppearAnim_ = 1.0f;

		// S字イージング
		float ease = startAppearAnim_ < 0.5f ? 2.0f * startAppearAnim_ * startAppearAnim_ : 1.0f - powf(-2.0f * startAppearAnim_ + 2.0f, 2.0f) / 2.0f;

		// フェードアウト
		float alpha = 1.0f - ease;

		startSprite_->SetColor({1, 1, 1, alpha});
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
		for (auto& block : scene->GetBlocks())
			block->Update();
		scene->GetPlayer()->Update();
		for (auto& enemy : scene->GetEnemies())
			enemy->Update();

		return;
	} else {
		// 落下アニメーションが終わった瞬間にSTART!!へ
		readyPhase_ = ReadyStatePhase::Start;
		readyTimer_ = 0.0f;
	}

	// START!!表示
	if (readyPhase_ == ReadyStatePhase::Start) {
		readyTimer_ += deltaTime;
		if (readyTimer_ > 0.7f) {
			readyPhase_ = ReadyStatePhase::None;
			readyTimer_ = 0.0f;
		}
	}

	readySprite_->Update();
	startSprite_->Update();
	restartSprite_->Update();

	// アニメーション終了後は全オブジェクト通常更新
	for (auto& block : scene->GetBlocks())
		block->Update();
	scene->GetPlayer()->Update();
	for (auto& enemy : scene->GetEnemies())
		enemy->Update();
	for (auto& tile : scene->GetTiles())
		tile->Update();
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
}

void StageReadyState::SpriteDraw(StageScene* scene) {
	if (readyPhase_ == ReadyStatePhase::Ready && readySprite_) {
		readySprite_->Draw();
	} else if (readyPhase_ == ReadyStatePhase::Start && startSprite_) {
		startSprite_->Draw();
	}
	restartSprite_->Draw();
}

void StageReadyState::ImGuiDraw(StageScene* scene) {
	// カメラ操作用UI
	if (scene->GetMainCamera()) {
		if (ImGui::CollapsingHeader("MainCamera")) {
			Vector3 camPos = scene->GetMainCamera()->GetTranslate();
			Vector3 camRot = scene->GetMainCamera()->GetRotation();
			Vector3 camScale = scene->GetMainCamera()->GetScale();

			if (ImGui::DragFloat3("Position", &camPos.x, 0.1f)) {
				scene->GetMainCamera()->SetTranslate(camPos);
				scene->GetMainCamera()->Update();
			}
			if (ImGui::DragFloat3("Rotation", &camRot.x, 0.1f)) {
				scene->GetMainCamera()->setRotation(camRot);
				scene->GetMainCamera()->Update();
			}
			if (ImGui::DragFloat3("Scale", &camScale.x, 0.1f)) {
				scene->GetMainCamera()->setScale(camScale);
				scene->GetMainCamera()->Update();
			}
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
	}
}

void StageReadyState::ParticleDraw(StageScene* scene) {}
