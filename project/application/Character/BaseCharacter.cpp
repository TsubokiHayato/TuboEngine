#include "BaseCharacter.h"

BaseCharacter::BaseCharacter() {}

BaseCharacter::~BaseCharacter() {}

void BaseCharacter::Initialize() {}

void BaseCharacter::Update() {}

void BaseCharacter::Draw() {}

void BaseCharacter::OnCollision(Collider* other) {}

TuboEngine::Math::Vector3 BaseCharacter::GetCenterPosition() const { return TuboEngine::Math::Vector3(); }
