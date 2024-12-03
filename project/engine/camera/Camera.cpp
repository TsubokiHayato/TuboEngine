#include "Camera.h"

void Camera::Update()
{
	worldMatrix_ = MakeAffineMatrix(transform_.translate, transform_.rotate, transform_.scale);
	viewMatrix_ = Inverse(worldMatrix_);
}
