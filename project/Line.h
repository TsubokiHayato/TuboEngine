#pragma once
#include "Transform.h"
#include "TransformationMatrix.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <wrl/client.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")

struct LineVertex {
	Vector3 position;
	Vector4 color;
};

class Camera;
class LineCommon;
class Line {
public:
	void Initialize(LineCommon* lineCommon);
	void Update();
	void Draw();
	void ClearLines();
	void DrawLine(const Vector3& start, const Vector3& end, const Vector4& color);

private:
	void CreateVertexBuffer();
	void CreateTransformationMatrixBuffer();

public:
	void SetTransform(const Transform& transform) { transform_ = transform; }
	Transform GetTransform() const { return transform_; }
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	const Vector3& GetScale() const { return transform_.scale; }
	void SetRotation(const Vector3& rotate) { transform_.rotate = rotate; }
	const Vector3& GetRotation() const { return transform_.rotate; }
	void SetPosition(const Vector3& translate) { transform_.translate = translate; }
	const Vector3& GetPosition() const { return transform_.translate; }
	void SetCamera(Camera* camera) { this->camera_ = camera; }

private:
	LineCommon* lineCommon_ = nullptr;
	std::vector<LineVertex> vertices_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = {};
	Microsoft::WRL::ComPtr<ID3D12Resource> transfomationMatrixBuffer_;
	TransformationMatrix* transformationMatrixData_ = nullptr;
	Transform transform_ = {};
	Camera* camera_ = nullptr;
};
