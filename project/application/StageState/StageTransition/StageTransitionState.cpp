#include "StageTransitionState.h"
#include "StageScene.h"
#include "Stage/StageManager.h"
#include "Character/Player/Player.h"
#include "Camera/FollowTopDownCamera.h"
#include "LineManager.h"
#include <algorithm>

using TuboEngine::Math::Vector3;

namespace {
float LengthSq(const Vector3& a, const Vector3& b) {
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	float dz = a.z - b.z;
	return dx * dx + dy * dy + dz * dz;
}
}

float StageTransitionState::EaseInOutQuad(float t) {
	if (t < 0.5f) {
		return 2.0f * t * t;
	}
	return 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}

void StageTransitionState::Enter(StageScene* scene) {
	initialized_ = false;
	if (!scene) return;

	Player* player = scene->GetPlayer();
	StageManager* stageMgr = scene->GetStageManager();
	if (!player || !stageMgr) return;

	// プレイヤーは既に kExit に到達している。ここが出発点。
	startPos_ = player->GetPosition();

	// 次のチャンクに進める
	if (!stageMgr->AdvanceToNextChunk()) {
		// 次のチャンクが無い → 全ステージクリア
		if (auto* mgr = scene->GetStageStateManager()) {
			mgr->ChangeState(StageType::StageClear, scene);
		}
		return;
	}

	// 次のチャンクの Entrance を目的地として取得
	int mainIndex = stageMgr->GetMainChunkIndex();
	const auto& insts = stageMgr->GetStageInstances();
	if (mainIndex < 0 || mainIndex >= static_cast<int>(insts.size())) return;

	const auto& nextInst = insts[mainIndex];
	MapChipField* nextField = nextInst.field.get();
	if (!nextField) return;

	// 次チャンクの Entrance 位置を取得
	auto entrances = nextField->GetChipPositions(MapChipType::kEntrance);
	if (!entrances.empty()) {
		targetPos_ = entrances.front();
	} else if (nextInst.playerMapX >= 0 && nextInst.playerMapY >= 0) {
		targetPos_ = nextField->GetMapChipPositionByIndex(
			static_cast<uint32_t>(nextInst.playerMapX),
			static_cast<uint32_t>(nextInst.playerMapY));
	} else {
		targetPos_ = nextInst.origin;
	}

	// 次チャンクの MapChipField をプレイヤーに設定
	player->SetMapChipField(nextField);

	// プレイヤー操作ロック
	player->SetMovementLocked(true);

	startTime_ = std::chrono::steady_clock::now();
	initialized_ = true;
}

void StageTransitionState::Update(StageScene* scene) {
	if (!scene) return;
	if (!initialized_) return;

	Player* player = scene->GetPlayer();
	if (!player) return;
	StageManager* stageMgr = scene->GetStageManager();
	FollowTopDownCamera* follow = scene->GetFollowCamera();
	// --- ステージオブジェクトやカメラは通常どおり更新してアニメーションとして見せる ---
	if (stageMgr && follow) {
		// Enemy 側で攻撃・移動しないようにするため、Update 前に簡易フラグを立てる等の制御を入れたい場合は、
		// 今後 StageManager/Enemy 側に専用APIを追加して対応する。
		// 現時点では Update 自体は行い、敵AI側で「Transition 中は攻撃・移動しない」分岐を入れておく想定。
		stageMgr->Update(player, follow);
	}
	if (follow) {
		follow->Update();
	}

	auto now = std::chrono::steady_clock::now();
	float elapsed = std::chrono::duration<float>(now - startTime_).count();
	float t = std::clamp(elapsed / durationSec_, 0.0f, 1.0f);
	float eased = EaseInOutQuad(t);

	// 位置イージング（XY平面）+ ジャンプ（Z軸の放物線）
	// sin(t * PI) で 0→1→0 の弧を描く（t=0.5 で頂点）
	float jumpArc = std::sin(t * 3.14159265f) * jumpHeight_;
	Vector3 newPos = {
		startPos_.x + (targetPos_.x - startPos_.x) * eased,
		startPos_.y + (targetPos_.y - startPos_.y) * eased,
		startPos_.z + jumpArc,  // Z軸にジャンプの弧を加算
	};
	player->SetPosition(newPos);

	// 向きも目標地点方向へ合わせる（XY 平面で計算。Player.cppにあわせる）
	Vector3 dir = { targetPos_.x - newPos.x, targetPos_.y - newPos.y, 0.0f };
	if (dir.x != 0.0f || dir.y != 0.0f) {
		float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
		dir.x /= len;
		dir.y /= len;
		float angle = std::atan2(dir.x, -dir.y);
		// Player.cpp の Rotate() と同じように Z軸回転を設定 (モデルの初期姿勢 3.12f 等は GetRotation に含まれるため上書きに注意しつつ、Zのみ更新)
		Vector3 currentRot = player->GetRotation();
		currentRot.z = 3.12f + angle;
		player->SetRotation(currentRot);
	}

	// プレイヤーの3Dモデルを実際に動かすために見た目だけ更新する
	// （ゲームロジックは一切実行しない。移動・射撃・自動操作すべてスキップ）
	player->UpdateVisualOnly();

	// カメラもプレイヤーに追従させる
	FollowTopDownCamera* followCamera = scene->GetFollowCamera();
	if (followCamera) {
		followCamera->Update();
		LineManager::GetInstance()->SetDefaultCamera(followCamera->GetCamera());
	}

#ifdef USE_IMGUI
	// --- デバッグ: 遷移の出発点(赤)と目的地(マゼンタ)を表示 ---
	const float mh = 4.0f;
	Vector3 startTop = { startPos_.x, startPos_.y, startPos_.z + mh };
	Vector3 targetTop = { targetPos_.x, targetPos_.y, targetPos_.z + mh };
	// 出発点（現在チャンクの Exit）: 赤
	LineManager::GetInstance()->DrawLine(startPos_, startTop, { 1.0f, 0.3f, 0.3f, 1.0f });
	LineManager::GetInstance()->DrawSphere(startTop, 0.6f, { 1.0f, 0.3f, 0.3f, 1.0f });
	// 目的地（次のチャンクの Entrance）: マゼンタ
	LineManager::GetInstance()->DrawLine(targetPos_, targetTop, { 1.0f, 0.3f, 1.0f, 1.0f });
	LineManager::GetInstance()->DrawSphere(targetTop, 0.6f, { 1.0f, 0.3f, 1.0f, 1.0f });
	// 出発→目的地のライン（白）
	LineManager::GetInstance()->DrawLine(startTop, targetTop, { 1.0f, 1.0f, 1.0f, 0.8f });
#endif // USE_IMGUI

	if (t >= 1.0f) {
		// 目的地（次のチャンクの Entrance）に到達：操作ロック解除して Playing に戻る
		player->SetMovementLocked(false);
		player->SetPosition(targetPos_); // 正確に Entrance 上にスナップ
		player->UpdateVisualOnly();
		if (auto* mgr = scene->GetStageStateManager()) {
			mgr->ChangeState(StageType::Playing, scene);
		}
	}
}

void StageTransitionState::Exit(StageScene* scene) {
	(void)scene;
}

void StageTransitionState::Object3DDraw(StageScene* scene) {
	// Playing と同様に 3D を表示し続ける
	if (!scene) return;
	if (auto* stageMgr = scene->GetStageManager()) {
		stageMgr->Draw3D();
	}
	if (auto* sky = scene->GetSkyDome().get()) {
		sky->Draw();
	}
	if (auto* player = scene->GetPlayer()) {
		player->Draw();
	}
}

void StageTransitionState::SpriteDraw(StageScene* scene) {
	if (!scene) return;
	// 必要ならプレイヤー照準などを描画
	if (auto* player = scene->GetPlayer()) {
		player->ReticleDraw();
	}
}

void StageTransitionState::ImGuiDraw(StageScene* scene) {
	(void)scene;
}

void StageTransitionState::ParticleDraw(StageScene* scene) {
	if (!scene) return;
	if (auto* stageMgr = scene->GetStageManager()) {
		const auto& insts = stageMgr->GetStageInstances();
		for (const auto& inst : insts) {
			for (const auto& e : inst.enemies) {
				if (e) e->ParticleDraw();
			}
		}
	}
}
