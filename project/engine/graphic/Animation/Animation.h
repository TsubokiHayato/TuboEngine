#pragma once
#include "KeyFrame.h"
#include <Vector3.h>
#include <map>
#include <string>
#include <vector>
#include <assimp/scene.h> // Assimpのシーンデータを使用するために必要
#include <assimp/anim.h> // Assimpのアニメーションデータを使用するために必要
#include <assimp/Importer.hpp> // Assimpのインポーターを使用するために必要
#include"assert.h"


template<typename tValue>
struct AnimationCurve {
	std::vector<KeyFrame<tValue>> keyframes; // キーフレームのリスト
};

struct NodeAnimation {
	AnimationCurve<Vector3> translate; // 位置のアニメーションカーブ
	AnimationCurve<Quaternion> rotate; // 回転のアニメーションカーブ
	AnimationCurve<Vector3> scale;     // スケールのアニメーションカーブ
};

struct Animation {
	float duration = 0.0f; // アニメーションの総時間
	// NodeAnimationの集合。Node名で引けるようにしておく
	std::map<std::string, NodeAnimation> nodeAnimations; // ノードごとのアニメーション
};

Animation LoadAnimation(const std::string& directoryPath, const std::string& filename);
std::vector<std::string> GetAnimationNodeNames(const std::string& directoryPath, const std::string& filename);
