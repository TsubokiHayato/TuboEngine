#include "BaseBullet.h"

void BaseBullet::OnCollision(Collider* other) {}

TuboEngine::Math::Vector3 BaseBullet::GetCenterPosition() const { return position; }

void BaseBullet::ApplyTransformToObject3d() {
    if (!object3d) return;
    object3d->SetPosition(position);
    object3d->SetRotation(rotation);
    object3d->SetScale(scale);
    object3d->Update();
}
