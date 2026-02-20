#pragma once

#include "engine/graphic/2d/Sprite.h"

#include <memory>
#include <random>
#include <string>
#include <vector>

/// <summary>
/// 画面遷移用のブロックアニメーション。
///
/// 画面をブロック（Sprite）で敷き詰め、
/// - `Disappearing`: ブロックが消えていく（画面が見えていく）
/// - `Appearing`   : ブロックが現れていく（画面を覆う）
/// を行う。
///
/// `SetPhase()` で開始フェーズを指定し、`Update()` を呼び続ける。
/// 終了判定は `IsFinished()`。
/// </summary>
class SceneChangeAnimation {
public:
	/// <summary>
	/// アニメーションの進行状態。
	/// </summary>
	enum class Phase { Appearing, Disappearing, Finished };

	/// <summary>
	/// コンストラクタ。
	/// </summary>
	/// <param name="screenWidth">画面幅（px）</param>
	/// <param name="screenHeight">画面高さ（px）</param>
	/// <param name="blockSize">ブロックサイズ（px）</param>
	/// <param name="duration">全体の基準時間（秒）</param>
	/// <param name="blockTexturePath">ブロックに貼るテクスチャパス</param>
	SceneChangeAnimation(int screenWidth, int screenHeight, int blockSize, float duration,
		const std::string& blockTexturePath);

	/// <summary>
	/// 内部状態を初期化し、ブロックを再生成する。
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新。
	/// </summary>
	/// <param name="deltaTime">前フレームからの経過時間（秒）</param>
	void Update(float deltaTime);

	/// <summary>
	/// 描画。
	/// </summary>
	void Draw() const;

	/// <summary>
	/// アニメーションが終了しているか。
	/// </summary>
	bool IsFinished() const;

	/// <summary>
	/// S字イージング（0..1 → 0..1）。
	/// </summary>
	static float EaseInOut(float t);

	/// <summary>
	/// デバッグUI。
	/// </summary>
	void DrawImGui();

	Phase GetPhase() const { return m_phase; }

	/// <summary>
	/// フェーズを切り替えてアニメーションを開始する。
	/// </summary>
	void SetPhase(Phase phase);

private:
	/// <summary>
	/// ブロック1枚分の状態。
	/// </summary>
	struct BlockSprite {
		int x = 0;
		int y = 0;
		float alpha = 0.0f;  // 0:透明 / 1:不透明
		float delay = 0.0f;  // ブロックごとの開始遅延（秒）
		bool fading = false; // 予約（将来拡張用）
		std::unique_ptr<TuboEngine::Sprite> sprite;
	};

	int m_screenWidth = 0;
	int m_screenHeight = 0;
	int m_blockSize = 0;
	float m_duration = 0.0f;
	float m_elapsed = 0.0f;
	std::vector<BlockSprite> m_blocks;
	std::mt19937 m_rng;
	std::string m_blockTexturePath;

	Phase m_phase = Phase::Disappearing;

	// 表示（Appearing）パラメータ
	float m_appearScaleMin = 0.1f;
	float m_appearScaleMax = 1.0f;
	float m_appearRotMax = 2.0f;

	// 非表示（Disappearing）パラメータ
	float m_disappearScaleMin = 0.1f;
	float m_disappearScaleMax = 1.0f;
	float m_disappearRotMax = 2.0f;

	/// <summary>
	/// 画面全体のブロックスプライトを生成する。
	/// </summary>
	void InitializeBlocks();

	/// <summary>
	/// Appearing フェーズの更新。
	/// </summary>
	void UpdateAppearing(float deltaTime);

	/// <summary>
	/// Disappearing フェーズの更新。
	/// </summary>
	void UpdateDisappearing(float deltaTime);
};
