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
	Enemy,       // 敵
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
	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);
	
	uint32_t GetNumBlockVirtical() { return static_cast<uint32_t>(mapChipData_.data.size()); }
	uint32_t GetNumBlockHorizontal() { return mapChipData_.data.empty() ? 0 : static_cast<uint32_t>(mapChipData_.data[0].size()); }

	///--------------------------------------------------
	///				構造体
	///--------------------------------------------------

	// マップチップのインデックスセット
	struct IndexSet {
		uint32_t xIndex;
		uint32_t yIndex;
	};

	// マップチップの矩形領域
	struct Rect {
		float left;
		float right;
		float top;
		float bottom;
	};

	// 座標からマップチップのインデックスセットを取得
	IndexSet GetMapChipIndexSetByPosition(const Vector3& position) const;
	// インデックスから矩形領域を取得
	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);

	// インデックスでマップチップ種別を設定
	void SetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex, MapChipType mapChipType);

	// 指定座標が通行可能か判定
	bool IsWalkable(const Vector3& position) const;

	// 指定座標が通行不可（壁など）か判定
	bool IsBlocked(const Vector3& position) const;

	// 指定矩形領域（プレイヤーの四隅など）が衝突しているか判定
	bool IsRectBlocked(const Vector3& center, float width, float height) const;

	// ImGuiの描画処理
	void DrawImGui(const char* windowName = "MapChipField");

	///--------------------------------------------------
	///		ブロックサイズ変更用ゲッター・セッター
	///--------------------------------------------------
	static float GetBlockWidth() { return kBlockWidth_; }
	static float GetBlockHeight() { return kBlockHeight_; }
	static float GetBlockSize() { return kBlockSize_; }

	static void SetBlockWidth(float w) { kBlockWidth_ = w; }
	static void SetBlockHeight(float h) { kBlockHeight_ = h; }
	static void SetBlockSize(float s) { kBlockSize_ = s; }


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
};