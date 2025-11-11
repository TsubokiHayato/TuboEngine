#include "StagePlayingState.h"
#include "LineManager.h"
#include "StageScene.h"
#include <cmath> // 追加: sqrtf など

namespace {
// 便利関数
inline float Clamp01(float v){ return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }
inline Vector3 Add(const Vector3&a,const Vector3&b){ return {a.x+b.x,a.y+b.y,a.z+b.z}; }
inline Vector3 Sub(const Vector3&a,const Vector3&b){ return {a.x-b.x,a.y-b.y,a.z-b.z}; }
inline Vector3 Mul(const Vector3&a,float s){ return {a.x*s,a.y*s,a.z*s}; }
inline Vector3 Lerp(const Vector3&a,const Vector3&b,float t){ return Add(a, Mul(Sub(b,a), t)); }
inline float Length(const Vector3&v){ return std::sqrt(v.x*v.x+v.y*v.y+v.z*v.z); }
inline Vector3 Normalize(const Vector3&v){ float l=Length(v); return (l<=1e-6f)?Vector3{0,0,0}:Vector3{v.x/l,v.y/l,v.z/l}; }

// カメラ基底（Right/Up/Forward）をView行列から取得（行ベース前提）
struct CamBasis{ Vector3 right, up, forward; };
inline CamBasis GetCameraBasis(const Camera* cam){
    CamBasis b{};
    const Matrix4x4& view = cam->GetViewMatrix();
    b.right   = Normalize({view.m[0][0], view.m[0][1], view.m[0][2]});
    b.up      = Normalize({view.m[1][0], view.m[1][1], view.m[1][2]});
    b.forward = Normalize({view.m[2][0], view.m[2][1], view.m[2][2]});
    b.forward = Mul(b.forward, -1.0f); // 視線方向へ
    return b;
}

// HP比に応じた色（赤→黄→緑）
inline Vector4 HPColor(float ratio){
    ratio = Clamp01(ratio);
    if(ratio < 0.5f){
        float t = ratio / 0.5f;           // 0..1
        return {1.0f, t, 0.0f, 1.0f};     // (1,0,0)->(1,1,0)
    }else{
        float t = (ratio - 0.5f) / 0.5f;  // 0..1
        return {1.0f - t, 1.0f, 0.0f, 1.0f}; // (1,1,0)->(0,1,0)
    }
}

// 線のみでビルボードHPバーを描く（外枠＋ストライプ）
void DrawHPBarBillboard(const Vector3& center, float width, float height, float ratio,
                        const Vector4& outlineColor, const Vector4& fillColor, const Vector4& backColor,
                        const Camera* cam){
    if(!cam) return;

    CamBasis basis = GetCameraBasis(cam);
    Vector3 right = basis.right;
    Vector3 up    = basis.up;

    float hw = width * 0.5f;
    float hh = height * 0.5f;

    // 外枠の4頂点（左上→右上→右下→左下）
    Vector3 p0 = Add(Add(center, Mul(right, -hw)), Mul(up,  hh));
    Vector3 p1 = Add(Add(center, Mul(right,  hw)), Mul(up,  hh));
    Vector3 p2 = Add(Add(center, Mul(right,  hw)), Mul(up, -hh));
    Vector3 p3 = Add(Add(center, Mul(right, -hw)), Mul(up, -hh));

    // 背景ストライプ
    const int bgStripes = 8;
    for(int i=0;i<=bgStripes;++i){
        float t = static_cast<float>(i) / static_cast<float>(bgStripes);
        Vector3 a = Lerp(p0, p1, t);
        Vector3 b = Lerp(p3, p2, t);
        LineManager::GetInstance()->DrawLine(a, b, backColor);
    }

    // 充填ストライプ（ratioまで）
    float r = Clamp01(ratio);
    Vector3 rt = Lerp(p0, p1, r);
    Vector3 rb = Lerp(p3, p2, r);
    const int fillStripes = 16;
    for(int i=0;i<=fillStripes;++i){
        float t = static_cast<float>(i) / static_cast<float>(fillStripes);
        Vector3 a = Lerp(p0, rt, t);
        Vector3 b = Lerp(p3, rb, t);
        LineManager::GetInstance()->DrawLine(a, b, fillColor);
    }

    // 外枠
    LineManager::GetInstance()->DrawLine(p0, p1, outlineColor);
    LineManager::GetInstance()->DrawLine(p1, p2, outlineColor);
    LineManager::GetInstance()->DrawLine(p2, p3, outlineColor);
    LineManager::GetInstance()->DrawLine(p3, p0, outlineColor);
}
} // namespace

// StagePlayingState
void StagePlayingState::Enter(StageScene* scene) {
	ruleSprite_ = std::make_unique<Sprite>();
	ruleSprite_->Initialize("rule.png");
	ruleSprite_->SetPosition({0.0f, 0.0f});
	ruleSprite_->Update();

}

void StagePlayingState::Update(StageScene* scene) {

	///------------------------------------------------
	/// 各オブジェクトの取得
	///------------------------------------------------

	Player* player_ = scene->GetPlayer();
	MapChipField* mapChipField_ = scene->GetMapChipField();
	std::vector<std::unique_ptr<Block>>& blocks_ = scene->GetBlocks();
	std::vector<std::unique_ptr<Enemy>>& enemies = scene->GetEnemies();
	FollowTopDownCamera* followCamera = scene->GetFollowCamera();

	///------------------------------------------------
	/// 各オブジェクトの更新
	///------------------------------------------------


	/// ブロック ///
	for (auto& block : blocks_) {
		// カメラ設定
		block->SetCamera(followCamera->GetCamera());
		// 更新
		block->Update();
	}

	/// プレイヤー ///
	// カメラ設定
	player_->SetCamera(followCamera->GetCamera());
	player_->SetIsDontMove(false);
	player_->SetMapChipField(mapChipField_);
	// 更新
	player_->Update();

	/// 敵 ///
	for (auto& enemy : enemies) {
		// カメラ設定
		enemy->SetCamera(followCamera->GetCamera());
		// プレイヤー設定
		enemy->SetPlayer(player_);
		// マップチップフィールド設定
		enemy->SetMapChipField(mapChipField_);
		// 更新
		enemy->Update();
	}

	/// Tile///
	std::vector<std::unique_ptr<Tile>>& tiles_ = scene->GetTiles();
	for (auto& tile : tiles_) {
		// カメラ設定
		tile->SetCamera(followCamera->GetCamera());
		// 更新
		tile->Update();
	}

	/// カメラ ///
	// 更新
	followCamera->Update();

	
	
	///------------------------------------------------
	/// ゲームクリア判定
	///------------------------------------------------

	// 全ての敵が倒されたかチェック
	bool allEnemiesDefeated = true;
	for (const auto& enemy : enemies) {
		if (enemy->GetIsAllive()) {
			allEnemiesDefeated = false;
			break;
		}
	}
	if (allEnemiesDefeated && !enemies.empty()) {
		scene->GetStageStateManager()->ChangeState(StageType::StageClear, scene);
		return;
	}

	///------------------------------------------------
	/// ゲームオーバー判定
	///------------------------------------------------

	if (!player_->GetIsAllive()) {
		// プレイヤーが死亡したらゲームオーバーステートへ
		scene->GetStageStateManager()->ChangeState(StageType::GameOver, scene);
		return;
	}



	ruleSprite_->Update();
	

}

void StagePlayingState::Exit(StageScene* scene) {}

void StagePlayingState::Object3DDraw(StageScene* scene) {
	// 3Dオブジェクトの描画
	// スカイドーム描画
	scene->GetSkyDome()->Draw();

	// タイル描画
	std::vector<std::unique_ptr<Tile>>& tiles_ = scene->GetTiles();
	for (auto& tile : tiles_) {
		tile->Draw();
	}

	// ブロック描画
	std::vector<std::unique_ptr<Block>>& blocks_ = scene->GetBlocks();
	for (auto& block : blocks_) {
		block->Draw();
	}

	// プレイヤーの3Dオブジェクトを描画
	scene->GetPlayer()->Draw();

	// 敵の3Dオブジェクトを描画
	std::vector<std::unique_ptr<Enemy>>& enemies = scene->GetEnemies();
	for (auto& enemy : enemies) {
		enemy->Draw();
	}

	// ここからHPバー描画（DrawLine）
	Camera* cam = scene->GetFollowCamera()->GetCamera();
	if (cam) {
		// Player: 自身の少し上に表示
		{
			Player* p = scene->GetPlayer();
			const float maxHp = 5.0f; // 必要ならPlayer側のMaxHPに置換
			float ratio = Clamp01(static_cast<float>(p->GetHP()) / maxHp);

			Vector3 pos = p->GetPosition();
			pos.y += 2.2f; // プレイヤー頭上オフセット

			Vector4 fill = HPColor(ratio);
			Vector4 outline = {1,1,1,1};
			Vector4 back = {0.12f,0.12f,0.12f,1.0f};

			DrawHPBarBillboard(pos, /*width*/3.0f, /*height*/0.35f, ratio, outline, fill, back, cam);
		}

		// Enemies: 頭上に表示
		for (auto& e : enemies) {
			if (!e) continue;
			if (!e->GetIsAllive()) continue;

			const float maxHp = static_cast<float>(e->GetMaxHP()); // 100固定ならGetMaxHP()不要
			float ratio = Clamp01(static_cast<float>(e->GetHP()) / maxHp);

			Vector3 pos = e->GetPosition();
			pos.y += 2.5f; // 敵の頭上オフセット

			Vector4 fill = HPColor(ratio);
			Vector4 outline = {1,1,1,1};
			Vector4 back = {0.10f,0.10f,0.10f,1.0f};

			DrawHPBarBillboard(pos, /*width*/2.2f, /*height*/0.25f, ratio, outline, fill, back, cam);
		}
	}
}

void StagePlayingState::SpriteDraw(StageScene* scene) { 
	scene->GetPlayer()->ReticleDraw();
	ruleSprite_->Draw();
}

void StagePlayingState::ImGuiDraw(StageScene* scene) {

	scene->GetFollowCamera()->DrawImGui();
	scene->GetPlayer()->DrawImGui();

	std::vector<std::unique_ptr<Enemy>>& enemies = scene->GetEnemies();
	// EnemyのImgui
	for (auto& enemy : enemies) {
		enemy->DrawImGui();
	}

	std::vector<std::unique_ptr<Block>>& blocks_ = scene->GetBlocks();
	// ブロックのImGui
	for (auto& block : blocks_) {
		block->DrawImGui();
	}

	
}

void StagePlayingState::ParticleDraw(StageScene* scene) {

	std::vector<std::unique_ptr<Enemy>>& enemies = scene->GetEnemies();

	for (auto& enemy : enemies) {
		enemy->ParticleDraw();
	}
}
