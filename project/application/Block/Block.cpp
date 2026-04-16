#include "Block.h"

//------------------------------------------------------------------------------
// Block
//------------------------------------------------------------------------------

Block::Block() = default;

Block::~Block() = default;

void Block::Initialize(const TuboEngine::Math::Vector3& position, const TuboEngine::Math::Vector3& scale, const std::string& modelFileName) {
	// --- 受け取った初期状態を保持 ---
	position_ = position;
	scale_ = scale;
	rotation_ = {0.0f, 0.0f, 0.0f};

	// --- 3D描画実体を生成・初期化 ---
	object3d_ = std::make_unique<TuboEngine::Object3d>();
	object3d_->Initialize(modelFileName);

	// --- 初期状態を `Object3d` に反映 ---
	object3d_->SetPosition(position_);
	object3d_->SetScale(scale_);
	object3d_->SetRotation(rotation_);
}

void Block::Update() {
	// 初期化前に呼ばれても落ちないようにガード
	if (!object3d_) {
		return;
	}

	// `Object3d` の更新へ委譲
	object3d_->Update();
}

void Block::Draw() {
	// 初期化前に呼ばれても落ちないようにガード
	if (!object3d_) {
		return;
	}

	// `Object3d` の描画へ委譲
	object3d_->Draw();
}

void Block::DrawImGui(const char* windowName) {
	// 初期化前に呼ばれても落ちないようにガード
	if (!object3d_) {
		return;
	}

	// `Object3d` のデバッグUIへ委譲
	object3d_->DrawImGui(windowName);
}

void Block::SetPosition(const TuboEngine::Math::Vector3& pos) {
	// --- 状態を保持 ---
	position_ = pos;

	// --- 描画実体へ反映（生成済みの場合のみ）---
	if (object3d_) {
		object3d_->SetPosition(pos);
	}
}

void Block::SetScale(const TuboEngine::Math::Vector3& scale) {
	// --- 状態を保持 ---
	scale_ = scale;

	// --- 描画実体へ反映（生成済みの場合のみ）---
	if (object3d_) {
		object3d_->SetScale(scale);
	}
}

void Block::SetRotation(const TuboEngine::Math::Vector3& rot) {
	// --- 状態を保持 ---
	rotation_ = rot;

	// --- 描画実体へ反映（生成済みの場合のみ）---
	if (object3d_) {
		object3d_->SetRotation(rot);
	}
}

void Block::SetCamera(TuboEngine::Camera* camera) {
	// カメラ設定は `Object3d` に委譲。
	// 初期化前に呼ばれる可能性があるのでガードする。
	if (object3d_) {
		object3d_->SetCamera(camera);
	}
}