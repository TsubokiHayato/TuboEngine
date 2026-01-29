#pragma once

#include <memory>
#include <string>

#include "Input.h"
#include "Sprite.h"

class GuideUI {
public:
	void Initialize();
	void Update();
	void Draw();

	void SetActive(bool active) { isActive_ = active; }
	bool IsActive() const { return isActive_; }

	void SetPosition(const Vector2& leftBottom) { basePos_ = leftBottom; }
	const Vector2& GetPosition() const { return basePos_; }

private:
	void UpdateKeySpriteState(class Sprite* sprite, const std::string& normalTex, const std::string& outlineTex, bool pressed) const;
	const std::string& SelectMouseIconTexture(const Input::MouseMove& move) const;

private:
	bool isActive_ = true;
	Vector2 basePos_{ 20.0f, 720.0f - 20.0f }; // will be converted to left-bottom in Initialize

	std::unique_ptr<Sprite> labelMove_;
	std::unique_ptr<Sprite> labelDash_;
	std::unique_ptr<Sprite> labelAim_;
	std::unique_ptr<Sprite> labelShoot_;

	std::unique_ptr<Sprite> keyW_;
	std::unique_ptr<Sprite> keyA_;
	std::unique_ptr<Sprite> keyS_;
	std::unique_ptr<Sprite> keyD_;
	std::unique_ptr<Sprite> keySpace_;
	std::unique_ptr<Sprite> mouseIcon_;
	std::unique_ptr<Sprite> mouseShootIcon_;

	// texture paths
	std::string texW_;
	std::string texWOutline_;
	std::string texA_;
	std::string texAOutline_;
	std::string texS_;
	std::string texSOutline_;
	std::string texD_;
	std::string texDOutline_;
	std::string texSpace_;
	std::string texSpaceOutline_;
	std::string texMouseSmall_;
	std::string texMouseMove_;
	std::string texMouse_;
	std::string texMouseRight_;
};
