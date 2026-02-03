#include "Collider.h"
#include"LineManager.h"

/// -------------------------------------------------------------
///						　	初期化処理
/// -------------------------------------------------------------
void Application::Collider::Initialize() {}

/// -------------------------------------------------------------
///						　	 更新処理
/// -------------------------------------------------------------
void Application::Collider::Update() {}

/// -------------------------------------------------------------
///						　	 描画処理
/// -------------------------------------------------------------
void Application::Collider::Draw() {
	TuboEngine::Math::Vector3 center = GetCenterPosition();
	LineManager::GetInstance()->DrawSphere(center, radius_, defaultColor, 16);
}
