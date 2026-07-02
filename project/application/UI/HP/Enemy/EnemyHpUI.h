#pragma once
#include <memory>
#include <vector>
#include "Sprite.h"
#include "Vector3.h"
#include"Camera.h"
class Enemy;


/// <summary>
/// 敵の頭上に表示するHPバーUI。複数の敵のHPバーをまとめて管理する。
/// </summary>
class EnemyHpUI {
public:
    /// <summary>
    /// 初期化処理。
    /// </summary>
    void Initialize(const std::string& frameTexturePath, const std::string& fillTexturePath);
    // 全チャンクの敵をまとめて扱えるように Enemy* 配列を受け取る
	void Update(const std::vector<Enemy*>& enemies, TuboEngine::Camera* cam);
    void Draw();

    /// <summary>
    /// スケールを設定する。
    /// </summary>
    void SetScale(float s) { scale_ = s; }
    /// <summary>
    /// 表示間隔を設定する。
    /// </summary>
    void SetSpacing(float s) { spacing_ = s; }
    /// <summary>
    /// YOffset を設定する。
    /// </summary>
    void SetYOffset(float y) { yOffset_ = y; }

private:
    /// <summary>
    /// 敵1体分のHPバーの表示状態（スプライト・HP値・アニメーション進行度）。
    /// </summary>
    struct EnemyBar {
		std::vector<std::unique_ptr<TuboEngine::Sprite>> frames; // per-HP frame
		std::vector<std::unique_ptr<TuboEngine::Sprite>> fills;  // per-HP fill
        bool visible = false;
        int maxHp = 0;
        int currentHp = 0;
        float animatedHp = 0.0f; // animated HP for smooth shrink
		TuboEngine::Math::Vector2 basePos{};
    };

    std::vector<EnemyBar> bars_;
    std::string frameTex_;
    std::string fillTex_;

    float scale_ = 1.0f;
    float spacing_ = 36.0f; // pixel spacing between icons
    float yOffset_ = -40.0f; // screen-space vertical offset above enemy head
    float shrinkSpeed_ = 4.0f; // icons per second
};
