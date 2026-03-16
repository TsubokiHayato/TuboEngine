#include "StageScene.h"
#include "Camera/FollowTopDownCamera.h"
#include "Collider/CollisionManager.h"
#include "LineManager.h"
#include "ParticleManager.h" // 追加: パーティクル描画/更新
#include"SceneType.h"

#include "engine/input/Input.h" // 追加

// 静的メンバ定義
bool StageScene::isDemoMode = false;

namespace {
StageScene::StageBounds ComputeBoundsWorld(const TuboEngine::Math::Vector3& origin, const MapChipField& field) {
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

	TuboEngine::Math::Vector3 ComputeSpawnOriginFromCenter(const StageScene::StageBounds& center, const StageScene::StageBounds& nextAtOrigin,
		NeighborDir dir, float gapX, float gapY) {
		const float nextW = nextAtOrigin.right - nextAtOrigin.left;
		const float nextH = nextAtOrigin.top - nextAtOrigin.bottom;

		TuboEngine::Math::Vector3 origin{0.0f, 0.0f, 0.0f};

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
	    TuboEngine::Math::Vector3 p0{b.left, b.bottom, z};
	    TuboEngine::Math::Vector3 p1{b.right, b.bottom, z};
	    TuboEngine::Math::Vector3 p2{b.right, b.top, z};
	    TuboEngine::Math::Vector3 p3{b.left, b.top, z};
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

	// StageManager の生成と基本設定のみ行う（実ロードは ReadyState でカメラ初期化後に行う）
	stageManager_ = std::make_unique<StageManager>();
	stageManager_->Configure(10, 10, 15.0f);
	// ★ ここでチャンクIDごとのCSVを登録
	stageManager_->RegisterChunkCsv(1, "Resources/Stage/Stage1.csv");
	stageManager_->RegisterChunkCsv(2, "Resources/Stage/Stage2.csv");

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

	// デモモード初期化
	if (isDemoMode) {
		player_->SetAutoControlEnabled(true);
		// デモ中はUIを非表示にする設定などがあればここで行う
	}

	// HP UI (Player)
	hpUI_ = std::make_unique<HpUI>();
	TuboEngine::TextureManager::GetInstance()->LoadTexture("HPBarFrame.png");
	TuboEngine::TextureManager::GetInstance()->LoadTexture("HP.png");
	int maxHp = 5; // Player 初期HPに合わせる
	hpUI_->Initialize("HPBarFrame.png", "HP.png", maxHp);
	hpUI_->SetPosition({20.0f, 20.0f});
	hpUI_->SetSpacing(4.0f);
	hpUI_->SetScale(0.8f);
	hpUI_->SetAlignRight(false); // 左揃えに変更

	// Enemy HP UI
	enemyHpUI_ = std::make_unique<EnemyHpUI>();
	TuboEngine::TextureManager::GetInstance()->LoadTexture("HPBarFrame.png");
	TuboEngine::TextureManager::GetInstance()->LoadTexture("HP.png");
	enemyHpUI_->Initialize("HPBarFrame.png", "HP.png");
	enemyHpUI_->SetYOffset(-24.0f);
	enemyHpUI_->SetScale(0.45f);
	enemyHpUI_->SetSpacing(0.0f);

	// Guide UI (WASD)
	guideUI_ = std::make_unique<GuideUI>();
	guideUI_->Initialize();

	// Demo sprites (two textures expected: "DemoLabel.png" and "PressAny.png")
	TuboEngine::TextureManager::GetInstance()->LoadTexture("DemoLabel.png");
	TuboEngine::TextureManager::GetInstance()->LoadTexture("PressAny.png");
	demoLabelSprite_ = std::make_unique<TuboEngine::Sprite>();
	demoPressAnySprite_ = std::make_unique<TuboEngine::Sprite>();
	demoLabelSprite_->Initialize("DEMO/DemoLabel.png");
	demoPressAnySprite_->Initialize("DEMO/PressAny.png");
	// Anchor to center for both
	demoLabelSprite_->SetAnchorPoint({0.5f, 0.5f});
	demoPressAnySprite_->SetAnchorPoint({0.5f, 0.5f});
	// Start invisible
	demoLabelSprite_->SetColor({1.0f, 1.0f, 1.0f, 0.0f});
	demoPressAnySprite_->SetColor({1.0f, 1.0f, 1.0f, 0.0f});
	demoLabelSprite_->Update();
	demoPressAnySprite_->Update();
}

void StageScene::Update() {

	// Scene change animation update
	sceneChangeAnimation_->Update(1.0f / 60.0f);

	// StageManager によるステージ構成の更新
	if (stageManager_) {
		stageManager_->Update(player_.get(), followCamera.get());
	}

	// Always update state manager so states can respond to animation/request flags
	if (stateManager_) {
		stateManager_->Update(this);
	}

	// デフォルトカメラをFollowCameraに設定
	LineManager::GetInstance()->SetDefaultCamera(followCamera->GetCamera());
	// 衝突マネージャの更新
	collisionManager_->Update();
	// 全ての衝突をチェック
	CheckAllCollisions();

	// 追加: プレイヤーが存在すればパーティクル更新 (Trail 用)
	TuboEngine::ParticleManager::GetInstance()->Update(1.0f / 60.0f, followCamera->GetCamera());

	// HP UI 更新
	if (hpUI_) { hpUI_->Update(player_.get()); }
	// Enemy HP UI 更新（エネミーに追従）"
	if (enemyHpUI_ && stageManager_ && followCamera) {
		std::vector<Enemy*> enemyPtrs;
		const auto& insts = stageManager_->GetStageInstances();
		for (const auto& inst : insts) {
			if (!inst.visible) continue;
			for (const auto& e : inst.enemies) {
				if (e && e->GetIsAlive()) {
					enemyPtrs.push_back(e.get());
				}
			}
		}
		enemyHpUI_->Update(enemyPtrs, followCamera->GetCamera());
	}
	// Guide UI 更新
	if (guideUI_) { guideUI_->Update(); }

	// --- Demo Mode Logic ---
	if (isDemoMode) {
		// Demo中は常に自動操作ONを維持する
		if (player_ && !player_->IsAutoControlEnabled()) {
			player_->SetAutoControlEnabled(true);
		}

		// 敵リストをプレイヤー(AutoController)に渡す
		if (player_ && player_->IsAutoControlEnabled()) {
			std::vector<Enemy*> enemyPtrs;
			if (stageManager_) {
				const auto& insts = stageManager_->GetStageInstances();
				for (const auto& inst : insts) {
					if (!inst.visible) continue;
					for (const auto& e : inst.enemies) {
						if (e && e->GetIsAlive()) {
							enemyPtrs.push_back(e.get());
						}
					}
				}
			}
			player_->SetEnemyList(enemyPtrs);
		}

		// 入力があればタイトルへ戻る
		bool pressAny = false;
		auto* input = TuboEngine::Input::GetInstance();
		if (input->TriggerKey(DIK_SPACE) || input->TriggerKey(DIK_RETURN) ||
			input->TriggerKey(DIK_Z) || input->TriggerKey(DIK_X) ||
			input->IsTriggerMouse(0) || input->IsTriggerMouse(1)) {
			pressAny = true;
		}

		if (pressAny) {
			// Use scene-change animation if available, otherwise fallback to immediate transition
			if (sceneChangeAnimation_) {
				if (!isRequestSceneChange) {
					// request animated transition to TITLE
					pendingNextSceneNo_ = TITLE;
					// clear demo flag now so other scenes don't treat next scene as demo
					StageScene::isDemoMode = false;
					sceneChangeAnimation_->SetPhase(SceneChangeAnimation::Phase::Appearing);
					isRequestSceneChange = true;
				}
			} else {
				// fallback: immediate change
				StageScene::isDemoMode = false;
				SceneManager::GetInstance()->ChangeScene(TITLE);
			}
		}
	}

	// SceneChangeAnimation がリクエストされており完了しているなら遷移を実行
	if (isRequestSceneChange && sceneChangeAnimation_ && sceneChangeAnimation_->IsFinished()) {
		if (pendingNextSceneNo_ >= 0) {
			int next = pendingNextSceneNo_;
			pendingNextSceneNo_ = -1;
			isRequestSceneChange = false;
			SceneManager::GetInstance()->ChangeScene(next);
		}
	}
}

void StageScene::Finalize() {}

void StageScene::Object3DDraw() {

	// ステージ(タイル/ブロック/敵)は StageManager からまとめて描画
	if (stageManager_) {
		stageManager_->Draw3D();
	}

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
	if (hpUI_ && !isDemoMode) { hpUI_->Draw(); }
	// Enemy HP UI 描画（追従）
	if (enemyHpUI_ && !isDemoMode) { enemyHpUI_->Draw(); }
	// Guide UI 描画
	if (guideUI_ && !isDemoMode) { guideUI_->Draw(); }

	// Demo sprites
	if (isDemoMode) {
		const float screenW = static_cast<float>(TuboEngine::WinApp::GetInstance()->GetClientWidth());
		const float screenH = static_cast<float>(TuboEngine::WinApp::GetInstance()->GetClientHeight());

		// Position demo label at upper center
		if (demoLabelSprite_) {
			float x = screenW * 0.5f;
			float y = screenH * 0.28f;
			demoLabelSprite_->SetPosition({x, y});
			// set a consistent size (scale as needed)
			// if texture large, adjust size; here we keep texture native size
			Vector4 c = demoLabelSprite_->GetColor();
			c.w = 1.0f; // visible
			demoLabelSprite_->SetColor(c);
			demoLabelSprite_->Update();
			demoLabelSprite_->Draw();
		}

		// Position press-any at lower center
		if (demoPressAnySprite_) {
			float x = screenW * 0.5f;
			float y = screenH * 0.72f;
			demoPressAnySprite_->SetPosition({x, y});
			Vector4 c = demoPressAnySprite_->GetColor();
			c.w = 1.0f; // visible
			demoPressAnySprite_->SetColor(c);
			demoPressAnySprite_->Update();
			demoPressAnySprite_->Draw();
		}
	}

	// アニメーション描画
	if (sceneChangeAnimation_) {
		sceneChangeAnimation_->Draw();
	}
}

void StageScene::ImGuiDraw() {

	
	#ifdef USE_IMGUI

	LineManager::GetInstance()->DrawImGui();
	stateManager_->DrawImGui(this);
	// StageManager の各チャンク情報を表示
	if (stageManager_) {
		stageManager_->DrawImGui();
	}

	#endif // USE_IMGUI

}

void StageScene::ParticleDraw() {
	
	if (stateManager_) {
		stateManager_->ParticleDraw(this);
	}
	// 追加: 全エミッター描画 (PlayerTrail 含む)
	TuboEngine::ParticleManager::GetInstance()->Draw();

#ifdef USE_IMGUI
	if (isDemoMode) {
		ImGui::SetNextWindowPos(ImVec2(TuboEngine::WinApp::GetInstance()->GetClientWidth() * 0.5f, 100.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowBgAlpha(0.0f); // 背景透明
	
	}
#endif
}
void StageScene::CheckAllCollisions() {
	collisionManager_->Reset();

	// ここで StageManager に一括登録させる
	if (stageManager_) {
		stageManager_->RegisterCollisions(collisionManager_.get(), player_.get());
	}

	collisionManager_->CheckAllCollisions();
}