#pragma once
#include "DirectXCommon.h"
#include "Line.h"
#include "LineCommon.h"
#include "SrvManager.h"
#include <memory>
#include <vector>

class LineManager {
public:
	static LineManager* GetInstance();
	void Initialize();
	void Finalize();
	void Update();
	void Draw();
	void DrawImGui();
	void ClearLines();
	void DrawLine(const Vector3& start, const Vector3& end, const Vector4& color);
	void DrawGrid(float gridSize, int divisions, const Vector4& color);
	void DrawSphere(const Vector3& center, float radius, const Vector4& color, int divisions = 32);
public:
	//Camera* GetDefaultCamera() { return lineCommon_->GetDefaultCamera(); }
	//void SetDefaultCamera(Camera* camera) { lineCommon_->SetDefaultCamera(camera); }
private:
	static LineManager* instance_;
	LineManager() = default;
	~LineManager() = default;
	LineManager(const LineManager&) = delete;
	LineManager& operator=(const LineManager&) = delete;
	std::unique_ptr<Line> line_;
	std::unique_ptr<LineCommon> lineCommon_;
	bool isDrawLine_ = true;
	bool isDrawGrid_ = true;
	float gridSize_ = 16.0f;
	int gridDivisions_ = 2;
	Vector4 gridColor_ = {0.0f, 0.0f, 0.0f, 1.0f};
	bool isDrawSphere_ = true;
};
