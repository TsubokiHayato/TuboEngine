#pragma once

#include "QuaternionTransform.h"
#include <map>
#include <optional>
#include <string>
#include <vector>
#include "Node.h"

class Skeleton {

public:
	struct Joint {
		QuaternionTransform transform;      // Transform
		Matrix4x4 localMatrix;              // ローカル行列
		Matrix4x4 skeletonSpaceMatrix;      // スケルトンスペースでの変換行列
		std::string name;                   // ジョイントの名前
		std::vector<int32_t> childrenIndex; // 子ジョイントのインデックスリスト
		int32_t index;                      // 自身のインデックス
		std::optional<int32_t> parentIndex; // 親ジョイントのインデックス（存在しない場合はstd::nullopt）
	};

	struct SkeletonData {
		int32_t rootIndex;                       // ルートジョイントのインデックス
		std::map<std::string, int32_t> jointMap; // ジョイント名とインデックスのマップ
		std::vector<Joint> joints;               // 所属しているジョイントのリスト
	};

public:
	// ジョイントの作成
	static int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints);

	// スケルトンの作成
	static SkeletonData CreateSkelton(const Node& rootNode);
	// スケルトンの更新
	static void Update(SkeletonData& skeleton);

	void DrawSkeletonLines(const Skeleton::SkeletonData& skeleton, const Vector4& color);

public:
	// ジョイントの更新
	void SetRootJoint(const Joint& joint) { rootJoint = joint; }
	Joint GetRootJoint() const { return rootJoint; }
	// スケルトンデータを取得
	SkeletonData GetSkeletonData() const { return skeletonData; }
	void SetSkeletonData(const SkeletonData& data) { skeletonData = data; }

private:
	SkeletonData skeletonData; // スケルトンデータ
	Joint rootJoint;           // ルートジョイント
};
