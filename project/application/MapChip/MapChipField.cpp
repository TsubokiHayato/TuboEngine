#include "MapChipField.h"
#include <assert.h>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include"ImGuiManager.h"

//--------------------------------------------------
// static変数の定義（ブロックサイズ）
//--------------------------------------------------
float MapChipField::kBlockWidth_ = 2.0f;
float MapChipField::kBlockHeight_ = 2.0f;
float MapChipField::kBlockSize_ = (kBlockWidth_ + kBlockHeight_) / 2.0f;


//--------------------------------------------------
// マップチップ種別テーブル（CSV値→MapChipType）
//--------------------------------------------------
namespace {
std::map<std::string, MapChipType> mapChipTable = {
    {"0", MapChipType::kBlank},
    {"1", MapChipType::kBlock},
    {"2", MapChipType::Player},
    {"3", MapChipType::Enemy },
    {"4", MapChipType::EnemyRush },
    {"5", MapChipType::EnemyShoot },
    {"8",MapChipType::EnemyMortar},
    {"9",MapChipType::EnemyCircus},
    {"6", MapChipType::kEntrance },
    {"7", MapChipType::kExit },
};
}

//--------------------------------------------------
// 座標からマップチップインデックスセットを取得
//--------------------------------------------------
MapChipField::IndexSet MapChipField::GetMapChipIndexSetByPosition(const TuboEngine::Math::Vector3& position) const {
    IndexSet indexSet = {};
    // 原点を考慮したローカル座標でインデックス計算
    float localX = position.x - origin_.x;
    float localY = position.y - origin_.y;
    indexSet.xIndex = static_cast<uint32_t>((localX + kBlockWidth_ / 2) / kBlockWidth_);
    indexSet.yIndex = static_cast<uint32_t>((localY + kBlockHeight_ / 2) / kBlockHeight_);
    return indexSet;
}

//--------------------------------------------------
// インデックスから矩形領域を取得
//--------------------------------------------------
MapChipField::Rect MapChipField::GetRectByIndex(uint32_t xIndex, uint32_t yIndex) const {
	TuboEngine::Math::Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);

	Rect rect;
	rect.left = center.x - kBlockWidth_ / 2.0f;
	rect.right = center.x + kBlockWidth_ / 2.0f;
	rect.top = center.y + kBlockHeight_ / 2.0f;
	rect.bottom = center.y - kBlockHeight_ / 2.0f;

	return rect;
}

//--------------------------------------------------
// マップチップデータをリセット
//--------------------------------------------------
void MapChipField::ResetMapChipData() {

	// マップチップデータをクリア
	mapChipData_.data.clear();

	mapChipData_.data.resize(kNumBlockVirtical);

	for (std::vector<MapChipType>& mapChipDataLine : mapChipData_.data) {
		mapChipDataLine.resize(kNumBlockHorizontal);
	}
}

//--------------------------------------------------
// CSVからマップチップデータを読み込む
//--------------------------------------------------
void MapChipField::LoadMapChipCsv(const std::string& filePath) {
	std::ifstream file(filePath);
	std::string line;
	mapChipData_.data.clear();

	while (std::getline(file, line)) {
		std::vector<MapChipType> row;
		std::stringstream ss(line);
		std::string cell;
		while (std::getline(ss, cell, ',')) {
			// マップチップ種別テーブルを利用して変換
			auto it = mapChipTable.find(cell);
			if (it != mapChipTable.end()) {
				row.push_back(it->second);
			} else {
				row.push_back(MapChipType::kBlank);
			}
		}
		mapChipData_.data.push_back(row);
	}
}
//--------------------------------------------------
// インデックスからマップチップ種別を取得（安全版）
//--------------------------------------------------
MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) const {

    // 実データで境界チェック
    if (yIndex >= mapChipData_.data.size()) {
        return MapChipType::kBlank;
    }
    const auto& row = mapChipData_.data[yIndex];
    if (xIndex >= row.size()) {
        return MapChipType::kBlank;
    }

    return row[xIndex];
}

//--------------------------------------------------
// インデックスからマップチップの座標を取得
//--------------------------------------------------
TuboEngine::Math::Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex)const {
	return TuboEngine::Math::Vector3(
        origin_.x + kBlockWidth_ * xIndex,      // X: 右へ+
        origin_.y + kBlockHeight_ * yIndex,     // Y: 上へ+
        origin_.z                        // Z: 原点に合わせる
    );
}

//--------------------------------------------------
// インデックスでマップチップ種別を設定
//--------------------------------------------------
void MapChipField::SetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex, MapChipType mapChipType) {
	// 実データで境界チェック（可変サイズCSV対応）
	if (yIndex >= mapChipData_.data.size()) {
		return;
	}
	auto& row = mapChipData_.data[yIndex];
	if (xIndex >= row.size()) {
		return;
	}
	row[xIndex] = mapChipType;
}

//--------------------------------------------------
// 指定座標が通行可能か判定
//--------------------------------------------------
bool MapChipField::IsWalkable(const TuboEngine::Math::Vector3& position) const {
    IndexSet index = GetMapChipIndexSetByPosition(position);
    MapChipType type = GetMapChipTypeByIndex(index.xIndex, index.yIndex);
    // kBlockは通行不可
    return type != MapChipType::kBlock;
}

//--------------------------------------------------
// 指定座標が通行不可（壁など）か判定（境界外は通行不可扱いにする例）
//--------------------------------------------------
bool MapChipField::IsBlocked(const TuboEngine::Math::Vector3& position) const {
    IndexSet index = GetMapChipIndexSetByPosition(position);

    // 実データで境界チェック（範囲外は通行可能扱いに変更）
    if (index.yIndex >= mapChipData_.data.size()) {
        return false;
    }
    const auto& row = mapChipData_.data[index.yIndex];
    if (index.xIndex >= row.size()) {
        return false;
    }

    MapChipType type = row[index.xIndex];
    return type == MapChipType::kBlock;
}

//--------------------------------------------------
// 指定矩形領域（プレイヤーの四隅など）が衝突しているか判定
//--------------------------------------------------
bool MapChipField::IsRectBlocked(const TuboEngine::Math::Vector3& center, float width, float height) const {
    // 矩形範囲をインデックス範囲に変換
    float halfW = width / 2.0f;
    float halfH = height / 2.0f;
    
    // 少し内側を判定対象にする（境界ギリギリでの誤検知防止）
    float margin = 0.01f;
    IndexSet minIdx = GetMapChipIndexSetByPosition({center.x - halfW + margin, center.y - halfH + margin, center.z});
    IndexSet maxIdx = GetMapChipIndexSetByPosition({center.x + halfW - margin, center.y + halfH - margin, center.z});

    // 範囲内の全チップをチェック
    for (uint32_t y = minIdx.yIndex; y <= maxIdx.yIndex; ++y) {
        if (y >= mapChipData_.data.size()) continue;
        const auto& row = mapChipData_.data[y];
        for (uint32_t x = minIdx.xIndex; x <= maxIdx.xIndex; ++x) {
            if (x >= row.size()) continue;
            if (row[x] == MapChipType::kBlock) {
                return true;
            }
        }
    }
    return false;
}

//--------------------------------------------------
// 指定タイプのチップのワールド座標リストを取得
//--------------------------------------------------
std::vector<TuboEngine::Math::Vector3> MapChipField::GetChipPositions(MapChipType type) const {
	std::vector<TuboEngine::Math::Vector3> result;
	for (uint32_t y = 0; y < mapChipData_.data.size(); ++y) {
		const auto& row = mapChipData_.data[y];
		for (uint32_t x = 0; x < row.size(); ++x) {
			if (row[x] == type) {
				// 座標計算では origin_ とブロックサイズを利用
				TuboEngine::Math::Vector3 pos(
					origin_.x + kBlockWidth_ * x,
					origin_.y + kBlockHeight_ * y,
					origin_.z);
				result.push_back(pos);
			}
		}
	}
	return result;
}

//--------------------------------------------------
// 指定座標の衝突法線を取得 (反射用)
//--------------------------------------------------
TuboEngine::Math::Vector3 MapChipField::GetCollisionNormal(const TuboEngine::Math::Vector3& pos, const TuboEngine::Math::Vector3& velocity) const {
	IndexSet index = GetMapChipIndexSetByPosition(pos);
	TuboEngine::Math::Vector3 center = GetMapChipPositionByIndex(index.xIndex, index.yIndex);
	TuboEngine::Math::Vector3 diff = pos - center;

	// X, Y のどちらの面に近いかで法線を決定
	if (std::abs(diff.x * kBlockHeight_) > std::abs(diff.y * kBlockWidth_)) {
		return { (diff.x > 0) ? 1.0f : -1.0f, 0.0f, 0.0f };
	} else {
		return { 0.0f, (diff.y > 0) ? 1.0f : -1.0f, 0.0f };
	}
}

//--------------------------------------------------
// ImGuiの描画処理
//--------------------------------------------------
void MapChipField::DrawImGui(const char* windowName) {

#ifdef USE_IMGUI
	ImGui::Begin(windowName);
	ImGui::SliderFloat("Block Width", &kBlockWidth_, 0.1f, 5.0f);
	ImGui::SliderFloat("Block Height", &kBlockHeight_, 0.1f, 5.0f);

    // --- ImGuiテーブル表示を必要な範囲だけに制限する例 ---
    const int showWidth = 20;   // 表示する横の最大数
    const int showHeight = 20;  // 表示する縦の最大数
    const int offsetX = 0;      // スクロールや表示開始位置に応じて調整
    const int offsetY = 0;

    if (ImGui::BeginTable("MapChipTable", showWidth, ImGuiTableFlags_Borders)) {
        for (int yIndex = offsetY; yIndex < offsetY + showHeight && yIndex < (int)kNumBlockVirtical; ++yIndex) {
            ImGui::TableNextRow();
            for (int xIndex = offsetX; xIndex < offsetX + showWidth && xIndex < (int)kNumBlockHorizontal; ++xIndex) {
                ImGui::TableSetColumnIndex(xIndex - offsetX);
                MapChipType mapChipType = GetMapChipTypeByIndex(xIndex, yIndex);
                const char* label = nullptr;
                ImVec4 color;
                switch (mapChipType) {
                    case MapChipType::kBlank:     label = "";   color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); break;
                    case MapChipType::kBlock:     label = "B";  color = ImVec4(0.3f, 0.3f, 0.8f, 1.0f); break;
                    case MapChipType::Player:     label = "P";  color = ImVec4(0.2f, 0.8f, 0.2f, 1.0f); break;
                    case MapChipType::Enemy:      label = "E";  color = ImVec4(0.8f, 0.2f, 0.2f, 1.0f); break;
                    case MapChipType::EnemyRush:  label = "ER"; color = ImVec4(1.0f, 0.4f, 0.2f, 1.0f); break;
                    case MapChipType::EnemyShoot: label = "ES"; color = ImVec4(0.9f, 0.6f, 0.2f, 1.0f); break;
					case MapChipType::EnemyMortar: label = "EM"; color = ImVec4(0.8f, 0.4f, 1.0f, 1.0f); break;
					case MapChipType::EnemyCircus: label = "EX"; color = ImVec4(1.0f, 0.0f, 1.0f, 1.0f); break;
					case MapChipType::kEntrance:  label = "IN"; color = ImVec4(0.2f, 0.6f, 1.0f, 1.0f); break;
					case MapChipType::kExit:      label = "OUT"; color = ImVec4(1.0f, 0.9f, 0.2f, 1.0f); break;
                    default:                      label = "?";  color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); break;
                }
                ImGui::PushStyleColor(ImGuiCol_Button, color);
                ImGui::PushID(yIndex * kNumBlockHorizontal + xIndex);
                if (ImGui::Button(label, ImVec2(24, 18))) {
                    // クリックで種類を順番に切り替え
                    MapChipType nextType = MapChipType::kBlank;
                    switch (mapChipType) {
                        case MapChipType::kBlank:     nextType = MapChipType::kBlock;      break;
                        case MapChipType::kBlock:     nextType = MapChipType::Player;      break;
                        case MapChipType::Player:     nextType = MapChipType::Enemy;       break;
                        case MapChipType::Enemy:      nextType = MapChipType::EnemyRush;   break;
                        case MapChipType::EnemyRush:  nextType = MapChipType::EnemyShoot;  break;
                        case MapChipType::EnemyShoot: nextType = MapChipType::kEntrance;   break;
					case MapChipType::kEntrance:  nextType = MapChipType::EnemyCircus; break;
					case MapChipType::EnemyCircus: nextType = MapChipType::kExit;     break;
					case MapChipType::kExit:      nextType = MapChipType::kBlank;    break;
                        default:                      nextType = MapChipType::kBlank;      break;
                    }
                    SetMapChipTypeByIndex(xIndex, yIndex, nextType);
                }
                ImGui::PopID();
                ImGui::PopStyleColor();
            }
        }
        ImGui::EndTable();
	}

	ImGui::End();
#endif // USE_IMGUI
}