#include "Line.h"
#include "Camera.h"
#include "DirectXCommon.h"
#include "LineManager.h"
#include "LineCommon.h"
#include "TransformationMatrix.h"
#define _USE_MATH_DEFINES
#include"MT_Matrix.h"
#include <math.h>

void Line::Initialize(LineCommon* lineCommon) {
	lineCommon_ = lineCommon;
	CreateVertexBuffer();
	CreateTransformationMatrixBuffer();
	transform_ = {
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };
	camera_ = lineCommon_->GetDefaultCamera();
}

void Line::Update() {
	camera_ = lineCommon_->GetDefaultCamera();
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	Matrix4x4 worldViewProjectionMatrix;
	if (camera_) {
		const Matrix4x4& viewMatrix = camera_->GetViewMatrix();
		const Matrix4x4& projectionMatrix = camera_->GetProjectionMatrix();
		Matrix4x4 worldViewMatrix = Multiply(worldMatrix, viewMatrix);
		worldViewProjectionMatrix = Multiply(worldViewMatrix, projectionMatrix);
	} else {
		worldViewProjectionMatrix = worldMatrix;
	}

	// 定数バッファへの書き込み
	transformationMatrixData_->WVP = worldViewProjectionMatrix;
	transformationMatrixData_->World = worldMatrix;
	//transformationMatrixData_->WorldInvTranspose = Inverse4x4(worldMatrix);
}



void Line::DrawLine(const Vector3& start, const Vector3& end, const Vector4& color) {
	vertices_.push_back({start, color});
	vertices_.push_back({end, color});
}

void Line::Draw() {
	if (vertices_.empty()) {
		return;
	}
	void* pData;
	vertexBuffer_->Map(0, nullptr, &pData);

	memcpy(pData, vertices_.data(), sizeof(LineVertex) * vertices_.size());

	vertexBuffer_->Unmap(0, nullptr);
	auto commandList = DirectXCommon::GetInstance()->GetCommandList();

	commandList->SetGraphicsRootConstantBufferView(0, transfomationMatrixBuffer_->GetGPUVirtualAddress()); // 修正: RootParameterIndexを0に変更

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

	commandList->DrawInstanced(static_cast<UINT>(vertices_.size()), 1, 0, 0);
	
}

void Line::ClearLines() {
	// ラインのクリア
	vertices_.clear();
}

void Line::CreateVertexBuffer() {
	//========================================
	// デバイスの取得
	auto device = DirectXCommon::GetInstance()->GetDevice();
	// バッファサイズ
	// NOTE: 1000本のラインを描画できるようにしている
	auto bufferSize = sizeof(LineVertex) * 100000;
	//========================================
	// バーテックスバッファの作成
	D3D12_HEAP_PROPERTIES heapProps = {};
	// ヒープタイプ
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	//========================================
	// リソースの設定
	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = bufferSize;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//========================================
	// リソースの作成
	device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer_));
	//========================================
	// バーテックスバッファビューの設定
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	// バイトサイズ
	vertexBufferView_.SizeInBytes = static_cast<UINT>(bufferSize);
	// ストライド
	vertexBufferView_.StrideInBytes = sizeof(LineVertex);
}

///=============================================================================
///
void Line::CreateTransformationMatrixBuffer() {
	// 定数バッファのサイズを 256 バイトの倍数に設定
	size_t bufferSize = (sizeof(TransformationMatrix) + 255) & ~255;
	transfomationMatrixBuffer_ = DirectXCommon::GetInstance()->CreateBufferResource(bufferSize);
	// 書き込み用変数
	TransformationMatrix transformationMatrix = {};
	// 書き込むためのアドレスを取得
	transfomationMatrixBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	// 書き込み
	transformationMatrix.WVP = MakeIdentity4x4();
	// 単位行列を書き込む
	*transformationMatrixData_ = transformationMatrix;
}
