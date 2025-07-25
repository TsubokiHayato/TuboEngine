#include "Block.h"

Block::Block() : position_(), scale_({1.0f, 1.0f, 1.0f}), rotation_() {}

Block::~Block() {}

void Block::Initialize(const Vector3& position, const Vector3& scale, const std::string& modelFileName) {
	position_ = position;
	scale_ = scale;
	rotation_ = {0.0f, 0.0f, 0.0f};
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(modelFileName);
	object3d_->SetPosition(position_);
	object3d_->SetScale(scale_);
	object3d_->SetRotation(rotation_);
}

void Block::Update() { object3d_->Update(); }

void Block::Draw() {
	if (object3d_) {
		object3d_->Draw();
	}
}

void Block::DrawImGui(const char* windowName) {
	if (object3d_) {
		object3d_->DrawImGui(windowName);
	}
}