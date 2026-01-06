#include "Tile.h"

Tile::Tile() : position_({0.0f,0.0f,-1.0f}), scale_({1.0f, 1.0f, 1.0f}), rotation_() {}

Tile::~Tile() {}

void Tile::Initialize(Vector3 position, const Vector3& scale, const std::string& modelFileName) {
	position_ = position;
	scale_ = scale;
	rotation_ = {0.0f, 0.0f, 0.0f};
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(modelFileName);
	object3d_->SetPosition(position_);
	object3d_->SetScale(scale_);
	object3d_->SetRotation(rotation_);
}

void Tile::Update() { object3d_->Update(); }

void Tile::Draw() {
	if (object3d_) {
		object3d_->Draw();
	}
}

void Tile::DrawImGui(const char* windowName) {
	if (object3d_) {
		object3d_->DrawImGui(windowName);
	}
}