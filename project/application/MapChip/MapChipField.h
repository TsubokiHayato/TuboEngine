#pragma once
#include "Vector3.h"
#include "string"
#include <cmath>
#include <cstdint>
#include <vector>

///--------------------------------------------------
/// マップチップ種別
///--------------------------------------------------
enum class MapChipType {
	kBlank,      // 空白
	kBlock,      // ブロック
	Player,      // プレイヤー
	Enemy,       // 敵（互換用の旧タイプ。今後は下の個別タイプを推奨）
	EnemyRush,   // 突進敵
	EnemyShoot,  // 射撃敵
	EnemyMortar, // 迫撃砲敵
	EnemyCircus, // 板野サーカス
	// --- ステージ遷移用マーカー ---
	kEntrance,   // ステージ入口
	kExit,       // ステージ出口（イージング遷移の目的地）

};

///--------------------------------------------------
/// マップチップデータ構造体
///--------------------------------------------------
struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

///--------------------------------------------------
/// マップチップフィールドクラス
///--------------------------------------------------
class MapChipField {
public:
	///--------------------------------------------------
	///				メンバ関数
	///--------------------------------------------------

	// マップチップデータをリセット
	void ResetMapChipData();
	// CSVからマップチップデータを読み込む
	void LoadMapChipCsv(const std::string& filePath);
	// インデックスからマップチップ種別を取得
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) const ;
	// インデックスからマップチップの座標を取得
	TuboEngine::Math::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) const;

	/// <summary>
	/// NumBlockVirtical を取得する。
	/// </summary>
	uint32_t GetNumBlockVirtical() const { return static_cast<uint32_t>(mapChipData_.data.size()); }
	/// <summary>
	/// NumBlockHorizontal を取得する。
	/// </summary>
	uint32_t GetNumBlockHorizontal() const { return mapChipData_.data.empty() ? 0 : static_cast<uint32_t>(mapChipData_.data[0].size()); }

	///--------------------------------------------------
	///				構造体
	///--------------------------------------------------

	/// <summary>
	/// マップチップのインデックス（x, y）の組。
	/// </summary>
	struct IndexSet {
		uint32_t xIndex;
		uint32_t yIndex;
	};

	/// <summary>
	/// マップチップの矩形領域。
	/// </summary>
	struct Rect {
		float left;
		float right;
		float top;
		float bottom;
	};

	// 座標からマップチップのインデックスセットを取得
	IndexSet GetMapChipIndexSetByPosition(const TuboEngine::Math::Vector3& position) const;
	// インデックスから矩形領域を取得
	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex) const;

	// インデックスでマップチップ種別を設定
	void SetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex, MapChipType mapChipType);

	// 指定座標が通行可能か判定
	bool IsWalkable(const TuboEngine::Math::Vector3& position) const;

	// 指定座標が通行不可（壁など）か判定
	bool IsBlocked(const TuboEngine::Math::Vector3& position) const;

	// 指定矩形領域（プレイヤーの四隅など）が衝突しているか判定
	bool IsRectBlocked(const TuboEngine::Math::Vector3& center, float width, float height) const;

	// 指定タイプのチップのワールド座標リストを取得（出口探索などに使用）
	std::vector<TuboEngine::Math::Vector3> GetChipPositions(MapChipType type) const;

	// 指定座標の衝突法線を取得 (反射用)
	TuboEngine::Math::Vector3 GetCollisionNormal(const TuboEngine::Math::Vector3& pos, const TuboEngine::Math::Vector3& velocity) const;

	// ImGuiの描画処理
	void DrawImGui(const char* windowName = "MapChipField");

	///--------------------------------------------------
	///		ブロックサイズ変更用ゲッター・セッター
	///--------------------------------------------------
	static float GetBlockWidth() { return kBlockWidth_; }
	static float GetBlockHeight() { return kBlockHeight_; }
	static float GetBlockSize() { return kBlockSize_; }

	/// <summary>
	/// BlockWidth を設定する。
	/// </summary>
	static void SetBlockWidth(float w) { kBlockWidth_ = w; }
	/// <summary>
	/// BlockHeight を設定する。
	/// </summary>
	static void SetBlockHeight(float h) { kBlockHeight_ = h; }
	/// <summary>
	/// BlockSize を設定する。
	/// </summary>
	static void SetBlockSize(float s) { kBlockSize_ = s; }

	// --- フィールド原点 ---
	void SetOrigin(const TuboEngine::Math::Vector3& origin) { origin_ = origin; }
	const TuboEngine::Math::Vector3& GetOrigin() const { return origin_; }


private:
	///--------------------------------------------------
	///				メンバ変数
	///--------------------------------------------------

	// 1ブロックのサイズ（static変数に変更）
	static float kBlockWidth_;
	static float kBlockHeight_;

	static float kBlockSize_; // kBlockWidth_とkBlockHeight_の平均値

	// ブロックの個数
	static inline const uint32_t kNumBlockVirtical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;

	// マップチップデータ
	MapChipData mapChipData_;

	// フィールド原点
	TuboEngine::Math::Vector3 origin_{0.0f, 0.0f, 0.0f};
};