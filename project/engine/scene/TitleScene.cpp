#include "TitleScene.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "LineManager.h"
#include "SceneManager.h"
#include "TextureManager.h"
#include "numbers"
#include <algorithm> // std::clamp用
#include <cmath>     // 追加

void TitleScene::Initialize() {


	

	// カメラ（3D向けに調整）
	camera = std::make_unique<Camera>();
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);

	// Line 描画用にデフォルトカメラをセット（ここで一度登録しておく）
	LineManager::GetInstance()->SetDefaultCamera(camera.get());

	titleUI = std::make_unique<TitleUI>();
	titleUI->Initialize();

	// Scene change animation
	sceneChangeAnimation = std::make_unique<SceneChangeAnimation>(1280, 720, 80, 1.5f, "barrier.png");
	sceneChangeAnimation->Initialize();
	isRequestSceneChange = false;

	// プレイヤー初期化（タイトル用）
	player_ = std::make_unique<Player>();
	player_->Initialize();
	// タイトルでは自動操作させない
	player_->SetDontMove(true);
	// カメラをセット（描画用）
	player_->SetCamera(camera.get());
	// 初期は画面左外から入ってくる位置にセット（ワールド座標系前提）
	player_->SetPosition({ -6.0f, 0.0f, 0.0f }); // 左端スタート
	player_->SetScale({ 1.0f, 1.0f, 1.0f });
	player_->Update();

	playerIntroDone_ = false;
	playerIntroTimer_ = 0.0f;
	playerIdleTime_ = 0.0f;

	// 背景タイマー
	time_ = 0.0f;
}

void TitleScene::Update() {
	// 固定60FPS想定（必要なら dt を取る実装に変更）
	const float dt = 1.0f / 60.0f;
	time_ += dt;


	// Player アニメーション: Intro -> Idle (呼吸・小さな振り向き・たまに hop)
	if (player_) {
		playerIntroTimer_ += (!playerIntroDone_) ? dt : 0.0f;

		Vector3 startPos = { -6.0f, 0.0f, 0.0f };
		Vector3 targetPos = { 0.0f, 0.0f, 0.0f };

		Vector3 pos = player_->GetPosition();
		Vector3 rot = {};
		Vector3 scl = player_->GetScale();

		// Intro: 左からゆっくり歩いてくる
		if (!playerIntroDone_) {
			float t = std::clamp(playerIntroTimer_ / playerIntroDuration_, 0.0f, 1.0f);
			// イーズ（ややスムーズに）
			float ease = 1.0f - std::pow(1.0f - t, 3.0f);
			pos.x = startPos.x + (targetPos.x - startPos.x) * ease;
			// 少し前進（z方向）の演出
			pos.z = startPos.z + (targetPos.z - startPos.z) * ease;
			// ほんの少し回転して歩く感じ
			rot.y = 0.25f * std::sin(ease * 3.14f);
			// 歩き終わったら完了フラグ
			if (t >= 1.0f) {
				playerIntroDone_ = true;
				playerIdleTime_ = 0.0f;
			}
		}
		else {
			// Idle ループ: 呼吸 (上下のゆっくりな bob) + 小さな左右体の揺れ + ときどき hop
			playerIdleTime_ += dt;
			// 呼吸ボブ
			float breathe = 0.14f * std::sin(playerIdleTime_ * 1.6f); // y振幅
			// 小さな前後スウェイ
			float sway = 0.06f * std::sin(playerIdleTime_ * 0.8f);
			
			pos = targetPos;
			pos.y += breathe;
			pos.z += sway;

			// 体のそよぎ（Y軸回転でわずかに振り向く）
			rot.y = 0.12f * std::sin(playerIdleTime_ * 0.9f);

			// 呼吸に合わせて少しスケール伸縮（スクワッシュ/ストレッチ）
			float breathScale = 1.0f + 0.03f * std::sin(playerIdleTime_ * 1.6f);
			scl = { breathScale, 1.0f / breathScale, breathScale };
		}

		player_->SetPosition(pos);
		player_->SetRotation(rot);
		player_->SetScale(scl);
		// 描画用に内部更新
		player_->Update();
	}

	// カメラ更新
	camera->SetTranslate(cameraPosition);
	camera->setRotation(cameraRotation);
	camera->setScale(cameraScale);
	camera->Update();

	// LineManager にカメラを渡し、Line の内部更新を行う（カメラが未設定だとスクリーン変換されない）
	LineManager::GetInstance()->SetDefaultCamera(camera.get());
	LineManager::GetInstance()->Update();

	// シーンチェンジアニメーション更新
	sceneChangeAnimation->Update(dt);

	// UI更新
	titleUI->Update();

	// UIからのシーン遷移要求を受け取って、演出開始
	if (!isRequestSceneChange && titleUI && titleUI->GetrRequestSceneChange_()) {
		if (sceneChangeAnimation->IsFinished()) {
			sceneChangeAnimation->SetPhase(SceneChangeAnimation::Phase::Appearing);
			isRequestSceneChange = true;
		}
	}

	// シーン遷移完了判定（UIの要求に応じて遷移）
	if (isRequestSceneChange && sceneChangeAnimation->IsFinished()) {
		int next = TITLE;
		if (titleUI) {
			switch (titleUI->GetNextSceneType()) {
			case SceneType::Select:
				next = STAGE;
				break;
			case SceneType::Tutorial:
				next = TUTORIAL;
				break;
			default:
				next = TITLE;
				break;
			}
			titleUI->ClearSceneChangeRequest();
		}
		SceneManager::GetInstance()->ChangeScene(next);
		isRequestSceneChange = false;
	}

	// 背景アニメ（時間のみで更新、Object3DDraw で参照）
}

void TitleScene::Finalize() {}

void TitleScene::Object3DDraw() {

	// 毎フレーム以前のラインをクリアしてから描画する（重複や残像を防ぐ）
	LineManager::GetInstance()->ClearLines();

	// ==== 背景アニメーション（線形で矩形を流すアニメーション） ====
	// 意図: 画面上を複数行で矩形フレームが一定速度で横移動する
	const int rectRows = 4;   // 矩形を並べる行数
	const int rectCount = 10; // 行あたりの矩形数（ループ用、実際は spacing で配置）
	const float screenW = 1280.0f;
	const float screenH = 720.0f;

	const float rectW = 220.0f;   // 矩形幅（ピクセル）
	const float rectH = 84.0f;    // 矩形高さ（ピクセル）
	const float spacing = 240.0f; // 矩形間隔（ピクセル）
	const float speed = 120.0f;   // 右方向への移動速度（ピクセル/秒）

	// global time を使って横方向オフセットを計算
	const float globalOffset = std::fmod(time_ * speed, spacing);

	for (int row = 0; row < rectRows; ++row) {
		// 行ごとに垂直位置と色を変化させる
		float tRow = static_cast<float>(row) / std::max(1, rectRows - 1);
		float baseY = 120.0f + tRow * (screenH - 240.0f); // 上下に広げる余白 120

		// 色のグラデーション（row によって変化）
		Vector4 color = {0.25f + 0.6f * tRow, 0.6f - 0.3f * tRow, 0.9f - 0.4f * tRow, 1.0f};

		// 各行で複数の矩形フレームを横方向に配置して移動させる
		for (int i = -1; i < rectCount + 1; ++i) {
			// 各矩形の基準 X を間隔で割り、その上で globalOffset を引いて流す
			float baseX = (i * spacing) - globalOffset;
			// 交互に左右の進行方向を逆にして視覚的に面白くする
			float direction = (row % 2 == 0) ? 1.0f : -1.0f;
			// 横移動のスケールを少し変えることで奥行き感
			float rowSpeedFactor = 1.0f + 0.15f * row;
			float x = screenW * 0.5f + (baseX * direction) * rowSpeedFactor;

			// 矩形の四隅を計算
			Vector3 p0 = {x - rectW * 0.5f, baseY - rectH * 0.5f, 0.0f};
			Vector3 p1 = {x + rectW * 0.5f, baseY - rectH * 0.5f, 0.0f};
			Vector3 p2 = {x + rectW * 0.5f, baseY + rectH * 0.5f, 0.0f};
			Vector3 p3 = {x - rectW * 0.5f, baseY + rectH * 0.5f, 0.0f};

			// スクリーン外の矩形は描画しても無駄なので簡易クリップ（少し余裕を持たせる）
			if (p1.x < -rectW || p0.x > screenW + rectW)
				continue;

			// 角に小さな装飾（少し回転するような印象を与えるため、行で位相ずらし）
			float wiggle = 6.0f * std::sin(time_ * 2.0f + row * 0.6f + i * 0.3f);
			Vector3 pp0 = {p0.x, p0.y, 0.0f};
			Vector3 pp1 = {p1.x, p1.y, 0.0f};
			Vector3 pp2 = {p2.x, p2.y, 0.0f};
			Vector3 pp3 = {p3.x, p3.y, 0.0f};

			// 矩形の枠線を描画
			LineManager::GetInstance()->DrawLine(pp0, pp1, color);
			LineManager::GetInstance()->DrawLine(pp1, pp2, color);
			LineManager::GetInstance()->DrawLine(pp2, pp3, color);
			LineManager::GetInstance()->DrawLine(pp3, pp0, color);

			// 内側に二重線で厚みを出す（ユニークな表現）
			Vector3 in0 = {(pp0.x + pp1.x) * 0.5f - rectW * 0.25f, (pp0.y + pp3.y) * 0.5f - rectH * 0.25f, 0.0f};
			Vector3 in1 = {(pp0.x + pp1.x) * 0.5f + rectW * 0.25f, (pp0.y + pp3.y) * 0.5f - rectH * 0.25f, 0.0f};
			Vector3 in2 = {(pp0.x + pp1.x) * 0.5f + rectW * 0.25f, (pp0.y + pp3.y) * 0.5f + rectH * 0.25f, 0.0f};
			Vector3 in3 = {(pp0.x + pp1.x) * 0.5f - rectW * 0.25f, (pp0.y + pp3.y) * 0.5f + rectH * 0.25f, 0.0f};

			// 少し薄めの色で内側ライン
			Vector4 innerColor = {color.x * 0.9f, color.y * 0.9f, color.z * 0.95f, 0.9f};
			LineManager::GetInstance()->DrawLine(in0, in1, innerColor);
			LineManager::GetInstance()->DrawLine(in1, in2, innerColor);
			LineManager::GetInstance()->DrawLine(in2, in3, innerColor);
			LineManager::GetInstance()->DrawLine(in3, in0, innerColor);
		}
	}

	// プレイヤー本体の描画（3D）
	if (player_) player_->Draw();
}

void TitleScene::SpriteDraw() {
	// ==== UI描画 ====
	titleUI->Draw();

	// アニメーション描画
	if (sceneChangeAnimation) {
		sceneChangeAnimation->Draw();
	}
}

void TitleScene::ImGuiDraw() {

#ifdef USE_IMGUI
	// カメラ
	ImGui::Begin("camera");
	ImGui::DragFloat3("Position", &cameraPosition.x, 0.01f);
	ImGui::DragFloat3("Rotation", &cameraRotation.x, 0.01f);
	ImGui::DragFloat3("Scale", &cameraScale.x, 0.01f);
	ImGui::End();

	if (sceneChangeAnimation) {
		sceneChangeAnimation->DrawImGui();
	}

	// プレイヤー調整（デバッグ用）
	if (player_) {
		ImGui::Begin("Player Title Anim");
		Vector3 ppos = player_->GetPosition();
		ImGui::Text("Position: %.2f, %.2f, %.2f", ppos.x, ppos.y, ppos.z);
		ImGui::Text("IntroDone: %d", playerIntroDone_ ? 1 : 0);
		ImGui::End();
	}
#endif // USE_IMGUI
}

void TitleScene::ParticleDraw() {}
