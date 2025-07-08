#include "BaseCharacter.h"

BaseCharacter::BaseCharacter() {}

BaseCharacter::~BaseCharacter() {}

void BaseCharacter::Initialize(Object3dCommon* object3dCommon) {}

void BaseCharacter::Update() {}

void BaseCharacter::Draw() {}

void BaseCharacter::OnCollision(Collider* other) {}

Vector3 BaseCharacter::GetCenterPosition() const { return Vector3(); }
