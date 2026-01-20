#include "StageScene.h"
#include "Camera/FollowTopDownCamera.h"
#include "Collider/CollisionManager.h"
#include "LineManager.h"
#include "ParticleManager.h" // 追加: パーティクル描画/更新

namespace {
	StageScene::StageBounds ComputeBoundsWorld(const Vector3& origin, const MapChipField& field) {
		const float w = static_cast<float>(field.GetNumBlockHorizontal()) * MapChipField::GetBlockWidth();
		const float h = static_cast<float>(field.GetNumBlockVirtical()) * MapChipField::GetBlockHeight();
		StageScene::StageBounds b;
		b.left = origin.x;
		b.right = origin.x + w;
		b.bottom = origin.y;
		b.top = origin.y + h;
		return b;
	}

	bool Intersects(const StageScene::StageBounds& a, const StageScene::StageBounds& b) {
		if (a.right <= b.left) return false;
		if (b.right <= a.left) return false;
		if (a.top <= b.bottom) return false;
		if (b.top <= a.bottom) return false;
		return true;
	}

	enum class NeighborDir {
		Up,
		Down,
		Left,
		Right,
	};

	Vector3 ComputeSpawnOriginFromCenter(const StageScene::StageBounds& center, const StageScene::StageBounds& nextAtOrigin,
		NeighborDir dir, float gapX, float gapY) {
		const float nextW = nextAtOrigin.right - nextAtOrigin.left;
		const float nextH = nextAtOrigin.top - nextAtOrigin.bottom;

		Vector3 origin{0.0f, 0.0f, 0.0f};

		switch (dir) {
		case NeighborDir::Right:
			origin.x = center.right + gapX;
			origin.y = center.bottom; // 下揃え
			break;
		case NeighborDir::Left:
			origin.x = center.left - gapX - nextW;
			origin.y = center.bottom;
			break;
		case NeighborDir::Up:
			// +Y が上
			origin.x = center.left; // 左揃え
			origin.y = center.top + gapY;
			break;
		case NeighborDir::Down:
			// -Y が下
			origin.x = center.left;
			origin.y = center.bottom - gapY - nextH;
			break;
		default:
			origin.x = center.right + gapX;
			origin.y = center.bottom;
			break;
		}

		return origin;
	}

	void DrawBounds(const StageScene::StageBounds& b, float z, const Vector4& color) {
		Vector3 p0{b.left, b.bottom, z};
		Vector3 p1{b.right, b.bottom, z};
		Vector3 p2{b.right, b.top, z};
		Vector3 p3{b.left, b.top, z};
		auto* lm = LineManager::GetInstance();
		lm->DrawLine(p0, p1, color);
		lm->DrawLine(p1, p2, color);
		lm->DrawLine(p2, p3, color);
		lm->DrawLine(p3, p0, color);
	}

	// 追加: 指定MapChipFieldを走査してコールバック（StageScene内ImGui用）
	template<typename Func>
	void ForEachMapChipField(MapChipField* field, Func func) {
		if (!field) {
			return;
		}
		for (uint32_t y = 0; y < field->GetNumBlockVirtical(); ++y) {
			for (uint32_t x = 0; x < field->GetNumBlockHorizontal(); ++x) {
				func(x, y, field->GetMapChipTypeByIndex(x, y));
			}
		}
	}
}

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
	tile_ = std::make_unique<Tile>();
	sceneChangeAnimation_ = std::make_unique<SceneChangeAnimation>(1280, 720, 80, 1.5f, "barrier.png");

	// 衝突マネージャの生成
	collisionManager_->Initialize();
	// ステートマネージャの生成
	stateManager_->Initialize(this);
	// シーンチェンジアニメーション初期化（シーン開始時はDisappearingで覆いを消す）
	sceneChangeAnimation_->Initialize();
	isRequestSceneChange = false;

	// 追加: TUTORIALシーンとして起動された場合はTutorialStateから開始
	if (GetSceneNo() == TUTORIAL && stateManager_) {
		stateManager_->ChangeState(StageType::Tutorial, this);
	}

	// Multi-stage (debug/editor): 初期ステージを2つ用意しておく
	stageInstances_.clear();
	{
		StageInstance st0;
		st0.csvPath = mapChipCsvFilePath_;
		st0.origin = {0.0f, 0.0f, 0.0f};
		stageInstances_.push_back(std::move(st0));

		StageInstance st1;
		st1.csvPath = mapChipCsvFilePath_; // とりあえず同じCSV（差し替えはImGui）
		st1.origin = {MapChipField::GetBlockWidth() * 110.0f, 0.0f, 0.0f};
		stageInstances_.push_back(std::move(st1));
	}

	// HP UI (Player)
	hpUI_ = std::make_unique<HpUI>();
	TextureManager::GetInstance()->LoadTexture("HPBarFrame.png");
	TextureManager::GetInstance()->LoadTexture("HP.png");
	int maxHp = 5; // Player 初期HPに合わせる
	hpUI_->Initialize("HPBarFrame.png", "HP.png", maxHp);
	hpUI_->SetPosition({20.0f, 20.0f});
	hpUI_->SetSpacing(4.0f);
	hpUI_->SetScale(0.8f);
	hpUI_->SetAlignRight(false); // 左揃えに変更

	// Enemy HP UI
	enemyHpUI_ = std::make_unique<EnemyHpUI>();
	TextureManager::GetInstance()->LoadTexture("HPBarFrame.png");
	TextureManager::GetInstance()->LoadTexture("HP.png");
	enemyHpUI_->Initialize("HPBarFrame.png", "HP.png");
	enemyHpUI_->SetYOffset(-24.0f);
	enemyHpUI_->SetScale(0.45f);
	enemyHpUI_->SetSpacing(0.0f);

	// Guide UI (WASD)
	guideUI_ = std::make_unique<GuideUI>();
	guideUI_->Initialize();
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

	// 追加: プレイヤーが存在すればパーティクル更新 (Trail 用)
	ParticleManager::GetInstance()->Update(1.0f/60.0f, followCamera->GetCamera());

	// HP UI 更新
	if (hpUI_) { hpUI_->Update(player_.get()); }
	// Enemy HP UI 更新（エネミーに追従）
	if (enemyHpUI_) { enemyHpUI_->Update(enemies, followCamera->GetCamera()); }
	// Guide UI 更新
	if (guideUI_) { guideUI_->Update(); }
}

void StageScene::Finalize() {}

void StageScene::Object3DDraw() {

	// Multi-stage bounds visualization (debug). Lines are cleared in some scenes; keep additive here.
	#ifdef USE_IMGUI
	if (useMultiStageLayout_) {
		// Ensure bounds are computed if fields are loaded
		for (auto& st : stageInstances_) {
			if (st.visible && st.field) {
				st.boundsWorld = ComputeBoundsWorld(st.origin, *st.field);
			}
		}
		// collision check
		for (size_t i = 0; i < stageInstances_.size(); ++i) {
			auto& a = stageInstances_[i];
			if (!a.visible || !a.field) continue;
			bool overlapped = false;
			for (size_t j = 0; j < stageInstances_.size(); ++j) {
				if (i == j) continue;
				auto& b = stageInstances_[j];
				if (!b.visible || !b.field) continue;
				if (Intersects(a.boundsWorld, b.boundsWorld)) {
					overlapped = true;
					break;
				}
			}
			DrawBounds(a.boundsWorld, -0.5f, overlapped ? Vector4{1.0f, 0.2f, 0.2f, 1.0f} : Vector4{0.2f, 1.0f, 0.2f, 1.0f});
		}
	}
	#endif

	// ステートマネージャの3Dオブジェクト描画
	if (stateManager_) {
		stateManager_->Object3DDraw(this);
	}
}

void StageScene::SpriteDraw() {
	
	// ステートマネージャのスプライト描画
	if (stateManager_) {
		stateManager_->SpriteDraw(this);
	}
	
	// HP UI 描画
	if (hpUI_) { hpUI_->Draw(); }
	// Enemy HP UI 描画（追従）
	if (enemyHpUI_) { enemyHpUI_->Draw(); }
	// Guide UI 描画
	if (guideUI_) { guideUI_->Draw(); }
	
	// アニメーション描画
	if (sceneChangeAnimation_) {
		sceneChangeAnimation_->Draw();
	}
}

void StageScene::ImGuiDraw() {

	
	#ifdef USE_IMGUI

	LineManager::GetInstance()->DrawImGui();
	stateManager_->DrawImGui(this);

	bool open = true;
	if (ImGui::Begin("Stage Layout (CSV)", &open)) {
		ImGui::Checkbox("Enable Multi Stage (debug)", &useMultiStageLayout_);

		static float snapCells = 1.0f;
		ImGui::SliderFloat("Snap (cells)", &snapCells, 0.0f, 10.0f, "%.1f");
		const float snapX = (snapCells <= 0.0f) ? 0.0f : MapChipField::GetBlockWidth() * snapCells;
		const float snapY = (snapCells <= 0.0f) ? 0.0f : MapChipField::GetBlockHeight() * snapCells;

		static float gapCellsX = 2.0f;
		static float gapCellsY = 2.0f;
		ImGui::SliderFloat("Gap X (cells)", &gapCellsX, 0.0f, 10.0f, "%.1f");
		ImGui::SliderFloat("Gap Y (cells)", &gapCellsY, 0.0f, 10.0f, "%.1f");
		const float gapX = MapChipField::GetBlockWidth() * gapCellsX;
		const float gapY = MapChipField::GetBlockHeight() * gapCellsY;

		if (ImGui::Button("Add Stage")) {
			StageInstance st;
			st.csvPath = mapChipCsvFilePath_;
			st.origin = {0.0f, 0.0f, 0.0f};
			stageInstances_.push_back(std::move(st));
		}
		ImGui::SameLine();
		if (ImGui::Button("Load All CSV")) {
			for (auto& st : stageInstances_) {
				st.field = std::make_unique<MapChipField>();
				st.field->LoadMapChipCsv(st.csvPath);
				st.boundsWorld = ComputeBoundsWorld(st.origin, *st.field);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Auto Arrange X")) {
			float cursorX = 0.0f;
			for (auto& st : stageInstances_) {
				if (!st.field) {
					st.field = std::make_unique<MapChipField>();
					st.field->LoadMapChipCsv(st.csvPath);
				}
				st.origin.x = cursorX;
				st.origin.y = 0.0f;
				st.boundsWorld = ComputeBoundsWorld(st.origin, *st.field);
				const float w = st.boundsWorld.right - st.boundsWorld.left;
				cursorX += w + gapX;
			}
		}

		ImGui::Separator();

		// 近傍配置UI（十字のみ / 中央は押せない）
		ImGui::Text("Spawn neighbor stage (cross)");
		if (!stageInstances_.empty()) {
			auto& centerSt = stageInstances_[0];
			if (!centerSt.field) {
				centerSt.field = std::make_unique<MapChipField>();
				centerSt.field->LoadMapChipCsv(centerSt.csvPath);
				centerSt.boundsWorld = ComputeBoundsWorld(centerSt.origin, *centerSt.field);
			}
			StageBounds centerB = centerSt.boundsWorld;

			// 次に置くステージCSV（新規作成用）
			static char spawnCsv[256] = "Resources/MapChip.csv";
			ImGui::InputText("Spawn CSV", spawnCsv, sizeof(spawnCsv));

			auto spawnNeighbor = [&](NeighborDir dir) {
				StageInstance next;
				next.csvPath = spawnCsv;
				next.field = std::make_unique<MapChipField>();
				next.field->LoadMapChipCsv(next.csvPath);

				StageBounds nextAtOrigin = ComputeBoundsWorld(Vector3{0, 0, 0}, *next.field);
				next.origin = ComputeSpawnOriginFromCenter(centerB, nextAtOrigin, dir, gapX, gapY);

				if (snapX > 0.0f) next.origin.x = std::round(next.origin.x / snapX) * snapX;
				if (snapY > 0.0f) next.origin.y = std::round(next.origin.y / snapY) * snapY;

				next.boundsWorld = ComputeBoundsWorld(next.origin, *next.field);

				for (auto& st : stageInstances_) {
					if (!st.field) {
						continue;
					}
					if (Intersects(next.boundsWorld, st.boundsWorld)) {
						return; // spawn failed
					}
				}

				stageInstances_.push_back(std::move(next));
			};

			const float cellSize = 28.0f;
			auto blankBtn = [&](const char* id, NeighborDir dir) {
				if (ImGui::Button(id, ImVec2(cellSize, cellSize))) {
					spawnNeighbor(dir);
				}
			};

			// Row1: [ ] [Up] [ ]
			ImGui::Button("##empty00", ImVec2(cellSize, cellSize)); ImGui::SameLine();
			blankBtn("##up", NeighborDir::Down); ImGui::SameLine();
			ImGui::Button("##empty02", ImVec2(cellSize, cellSize));

			// Row3: [ ] [Down] [ ]
			ImGui::Button("##empty20", ImVec2(cellSize, cellSize)); ImGui::SameLine();
			blankBtn("##down", NeighborDir::Up); ImGui::SameLine();
			ImGui::Button("##empty22", ImVec2(cellSize, cellSize));
		} else {
			ImGui::Text("No center stage. Add one first.");
		}

		ImGui::Separator();

		// 追加: プレビュー用オブジェクトの再生成（scene再起動なしで反映）
		ImGui::Text("Preview Objects");
		if (ImGui::Button("Rebuild Preview Objects")) {
			for (auto& st : stageInstances_) {
				if (!st.visible) {
					continue;
				}
				// フィールドが未ロードならロード
				if (!st.field) {
					st.field = std::make_unique<MapChipField>();
					st.field->LoadMapChipCsv(st.csvPath);
				}
				// Stage0は既存生成ロジックに任せる（ここでは1+のみ生成）
				if (&st == &stageInstances_[0]) {
					continue;
				}

				st.blocks.clear();
				st.enemies.clear();
				st.tile = std::make_unique<Tile>();

				Vector3 tilePos = st.origin;
				tilePos.z = -1.0f;
				st.tile->Initialize(tilePos, {1.0f, 1.0f, 1.0f}, "tile/tile30x30.obj");
				st.tile->SetCamera(followCamera->GetCamera());
				st.tile->Update();

				ForEachMapChipField(st.field.get(), [&](uint32_t x, uint32_t y, MapChipType type) {
					Vector3 pos = st.field->GetMapChipPositionByIndex(x, y) + st.origin;
					if (type == MapChipType::kBlock) {
						auto block = std::make_unique<Block>();
						block->Initialize(pos);
						block->SetCamera(followCamera->GetCamera());
						block->Update();
						st.blocks.push_back(std::move(block));
					} else if (type == MapChipType::Enemy || type == MapChipType::EnemyRush) {
						auto enemy = std::make_unique<RushEnemy>();
						enemy->Initialize();
						enemy->SetCamera(followCamera->GetCamera());
						enemy->SetPlayer(player_.get());
						enemy->SetPosition(pos);
						enemy->Update();
						st.enemies.push_back(std::move(enemy));
					} else if (type == MapChipType::EnemyShoot) {
						auto enemy = std::make_unique<Enemy>();
						enemy->Initialize();
						enemy->SetCamera(followCamera->GetCamera());
						enemy->SetPlayer(player_.get());
						enemy->SetPosition(pos);
						enemy->Update();
						st.enemies.push_back(std::move(enemy));
					}
				});
			}
		}

		ImGui::Separator();

		for (size_t i = 0; i < stageInstances_.size(); ++i) {
			auto& st = stageInstances_[i];
			ImGui::PushID(static_cast<int>(i));

			ImGui::Checkbox("Visible", &st.visible);
			ImGui::SameLine();
			ImGui::Text("Stage %zu", i);

			char buf[256]{};
			strncpy_s(buf, st.csvPath.c_str(), sizeof(buf) - 1);
			if (ImGui::InputText("CSV", buf, sizeof(buf))) {
				st.csvPath = buf;
			}

			Vector3 origin = st.origin;
			float o[2] = { origin.x, origin.y };
			if (ImGui::DragFloat2("Origin", o, 0.1f)) {
				origin.x = o[0];
				origin.y = o[1];
				if (snapX > 0.0f) origin.x = std::round(origin.x / snapX) * snapX;
				if (snapY > 0.0f) origin.y = std::round(origin.y / snapY) * snapY;
				st.origin = origin;
				if (st.field) {
					st.boundsWorld = ComputeBoundsWorld(st.origin, *st.field);
				}
			}

			if (ImGui::Button("Reload CSV")) {
				st.field = std::make_unique<MapChipField>();
				st.field->LoadMapChipCsv(st.csvPath);
				st.boundsWorld = ComputeBoundsWorld(st.origin, *st.field);
			}
			ImGui::SameLine();
			if (ImGui::Button("Remove")) {
				stageInstances_.erase(stageInstances_.begin() + static_cast<long long>(i));
				ImGui::PopID();
				break;
			}

			if (st.field) {
				ImGui::Text("Size (cells): %u x %u", st.field->GetNumBlockHorizontal(), st.field->GetNumBlockVirtical());
				ImGui::Text("Bounds: L%.1f R%.1f B%.1f T%.1f", st.boundsWorld.left, st.boundsWorld.right, st.boundsWorld.bottom, st.boundsWorld.top);
			}

			ImGui::Separator();
			ImGui::PopID();
		}

		ImGui::End();
	} else {
		ImGui::End();
	}

	#endif // USE_IMGUI

}

void StageScene::ParticleDraw() {
	
	if (stateManager_) {
		stateManager_->ParticleDraw(this);
	}
	// 追加: 全エミッター描画 (PlayerTrail 含む)
	ParticleManager::GetInstance()->Draw();
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