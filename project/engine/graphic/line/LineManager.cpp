#include "LineManager.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "ImGuiManager.h"

///----------------------------------------------------
/// シングルトンインスタンス初期化
///----------------------------------------------------
LineManager* LineManager::instance_ = nullptr;

///----------------------------------------------------
/// LineManagerインスタンス取得
///----------------------------------------------------
LineManager* LineManager::GetInstance() {
    if (instance_ == nullptr) {
        instance_ = new LineManager();
    }
    return instance_;
}

///----------------------------------------------------
/// LineManager初期化処理
///----------------------------------------------------
void LineManager::Initialize() {
    // Line描画共通処理クラス生成・初期化
    lineCommon_ = std::make_unique<LineCommon>();
    lineCommon_->Initialize();
    // Line描画クラス生成・初期化
    line_ = std::make_unique<Line>();
    line_->Initialize(lineCommon_.get());
}

///----------------------------------------------------
/// LineManager終了処理
///----------------------------------------------------
void LineManager::Finalize() {
    delete instance_;
    instance_ = nullptr;
}

///----------------------------------------------------
/// LineManager更新処理
///----------------------------------------------------
void LineManager::Update() {
   
    // Line描画クラス更新
    line_->Update();
}

///----------------------------------------------------
/// LineManager描画処理
///----------------------------------------------------
void LineManager::Draw() {
    // Line描画共通設定
    lineCommon_->DrawSettingsCommon();
    // Line描画
    line_->Draw();
    // ライン頂点情報クリア
    line_->ClearLines();
}

///----------------------------------------------------
/// ImGui描画処理
///----------------------------------------------------
void LineManager::DrawImGui() {
    ImGui::Begin("LineManager");
    ImGui::Checkbox("Line", &isDrawLine_);
    ImGui::Separator();
    ImGui::Checkbox("Grid", &isDrawGrid_);
    ImGui::SliderFloat("GridSize", &gridSize_, 1.0f, 10000.0f);
    ImGui::SliderInt("Divisions", &gridDivisions_, 1, 512);
    ImGui::Checkbox("Sphere", &isDrawSphere_);
    ImGui::ColorEdit4("Color", &gridColor_.x);
    ImGui::End();
}

///----------------------------------------------------
/// ライン頂点情報クリア
///----------------------------------------------------
void LineManager::ClearLines() {
    line_->ClearLines();
}

///----------------------------------------------------
/// ライン描画用頂点追加
///----------------------------------------------------
void LineManager::DrawLine(const Vector3& start, const Vector3& end, const Vector4& color) {
    if (!isDrawLine_) {
        return;
    }
    line_->DrawLine(start, end, color);
}

///----------------------------------------------------
/// グリッド描画
///----------------------------------------------------
void LineManager::DrawGrid(float gridSize, int divisions, const Vector4& color) {
    if (!isDrawGrid_ || divisions <= 0) {
        return;
    }
    float halfSize = gridSize * 0.5f;
    float step = gridSize / divisions;
    for (int i = 0; i <= divisions; ++i) {
        float offset = -halfSize + (i * step);
        DrawLine(Vector3(-halfSize, 0.0f, offset), Vector3(halfSize, 0.0f, offset), color);
        DrawLine(Vector3(offset, 0.0f, -halfSize), Vector3(offset, 0.0f, halfSize), color);
    }
}

///----------------------------------------------------
/// 球描画
///----------------------------------------------------
void LineManager::DrawSphere(const Vector3& center, float radius, const Vector4& color, int divisions) {
    if (!isDrawSphere_ || divisions <= 0) {
        return;
    }
    float angleStep = 2.0f * static_cast<float>(M_PI) / divisions;
    for (int i = 0; i < divisions; ++i) {
        float angle1 = angleStep * i;
        float angle2 = angleStep * (i + 1);
        DrawLine(Vector3(center.x + radius * cosf(angle1), center.y + radius * sinf(angle1), center.z), Vector3(center.x + radius * cosf(angle2), center.y + radius * sinf(angle2), center.z), color);
        DrawLine(Vector3(center.x + radius * cosf(angle1), center.y, center.z + radius * sinf(angle1)), Vector3(center.x + radius * cosf(angle2), center.y, center.z + radius * sinf(angle2)), color);
        DrawLine(Vector3(center.x, center.y + radius * cosf(angle1), center.z + radius * sinf(angle1)), Vector3(center.x, center.y + radius * cosf(angle2), center.z + radius * sinf(angle2)), color);
    }
    for (int lat = 1; lat < divisions / 2; ++lat) {
        float latAngle1 = static_cast<float>(M_PI) * lat / (divisions / 2);
        float latAngle2 = static_cast<float>(M_PI) * (lat + 1) / (divisions / 2);
        float r1 = radius * sinf(latAngle1);
        float r2 = radius * sinf(latAngle2);
        float y1 = center.y + radius * cosf(latAngle1);
        float y2 = center.y + radius * cosf(latAngle2);
        for (int i = 0; i < divisions; ++i) {
            float angle1 = angleStep * i;
            float angle2 = angleStep * (i + 1);
            DrawLine(Vector3(center.x + r1 * cosf(angle1), y1, center.z + r1 * sinf(angle1)), Vector3(center.x + r1 * cosf(angle2), y1, center.z + r1 * sinf(angle2)), color);
            DrawLine(Vector3(center.x + r2 * cosf(angle1), y2, center.z + r2 * sinf(angle1)), Vector3(center.x + r2 * cosf(angle2), y2, center.z + r2 * sinf(angle2)), color);
        }
    }
    for (int lon = 0; lon < divisions; ++lon) {
        float lonAngle = angleStep * lon;
        for (int lat = 0; lat <= divisions / 2; ++lat) {
            float latAngle = static_cast<float>(M_PI) * lat / (divisions / 2);
            float nextLatAngle = static_cast<float>(M_PI) * (lat + 1) / (divisions / 2);
            float r1 = radius * sinf(latAngle);
            float r2 = radius * sinf(nextLatAngle);
            float y1 = center.y + radius * cosf(latAngle);
            float y2 = center.y + radius * cosf(nextLatAngle);
            DrawLine(Vector3(center.x + r1 * cosf(lonAngle), y1, center.z + r1 * sinf(lonAngle)), Vector3(center.x + r2 * cosf(lonAngle), y2, center.z + r2 * sinf(lonAngle)), color);
        }
    }
}
