#include "Line.h"
#include "Camera.h"
#include "DirectXCommon.h"
#include "LineManager.h"
#include "LineCommon.h"
#include "TransformationMatrix.h"
#define _USE_MATH_DEFINES
#include "MT_Matrix.h"
#include <math.h>

///----------------------------------------------------
/// Lineクラスの初期化処理
///----------------------------------------------------
void Line::Initialize(LineCommon* lineCommon) {
    // Line描画の共通処理管理クラスをセット
    lineCommon_ = lineCommon;
    // 頂点バッファ生成
    CreateVertexBuffer();
    // 座標変換行列バッファ生成
    CreateTransformationMatrixBuffer();
    // Transformの初期化
    transform_ = {
        {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };
    // デフォルトカメラ取得
    camera_ = lineCommon_->GetDefaultCamera();
}

///----------------------------------------------------
/// Lineクラスの更新処理
///----------------------------------------------------
void Line::Update() {
    // デフォルトカメラ取得
    camera_ = lineCommon_->GetDefaultCamera();
    // ワールド行列作成
    Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    Matrix4x4 worldViewProjectionMatrix;
    if (camera_) {
        // ビュー・プロジェクション行列取得
        const Matrix4x4& viewMatrix = camera_->GetViewMatrix();
        const Matrix4x4& projectionMatrix = camera_->GetProjectionMatrix();
        // ワールド→ビュー→プロジェクション
        Matrix4x4 worldViewMatrix = Multiply(worldMatrix, viewMatrix);
        worldViewProjectionMatrix = Multiply(worldViewMatrix, projectionMatrix);
    } else {
        // カメラが無い場合はワールド行列のみ
        worldViewProjectionMatrix = worldMatrix;
    }
    // 定数バッファへの書き込み
    transformationMatrixData_->WVP = worldViewProjectionMatrix;
    transformationMatrixData_->World = worldMatrix;
    //transformationMatrixData_->WorldInvTranspose = Inverse4x4(worldMatrix);
}

///----------------------------------------------------
/// ライン描画用頂点追加
///----------------------------------------------------
void Line::DrawLine(const Vector3& start, const Vector3& end, const Vector4& color) {
    vertices_.push_back({start, color});
    vertices_.push_back({end, color});
}

///----------------------------------------------------
/// ライン描画処理
///----------------------------------------------------
void Line::Draw() {
    if (vertices_.empty()) {
        return;
    }
    // 頂点バッファへデータ転送
    void* pData;
    vertexBuffer_->Map(0, nullptr, &pData);
    memcpy(pData, vertices_.data(), sizeof(LineVertex) * vertices_.size());
    vertexBuffer_->Unmap(0, nullptr);
    auto commandList = DirectXCommon::GetInstance()->GetCommandList();
    // 定数バッファビュー設定
    commandList->SetGraphicsRootConstantBufferView(0, transfomationMatrixBuffer_->GetGPUVirtualAddress());
    // 頂点バッファビュー設定
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
    // ライン描画
    commandList->DrawInstanced(static_cast<UINT>(vertices_.size()), 1, 0, 0);
}

///----------------------------------------------------
/// ライン頂点情報のクリア
///----------------------------------------------------
void Line::ClearLines() {
    vertices_.clear();
}

///----------------------------------------------------
/// 頂点バッファ生成
///----------------------------------------------------
void Line::CreateVertexBuffer() {
    // デバイスの取得
    auto device = DirectXCommon::GetInstance()->GetDevice();
    // バッファサイズ（最大100000頂点分）
    auto bufferSize = sizeof(LineVertex) * 100000;
    // ヒーププロパティ設定（UPLOAD）
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    // リソース記述設定
    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = bufferSize;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    // バッファリソース生成
    device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer_));
    // バッファビュー設定
    vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<UINT>(bufferSize);
    vertexBufferView_.StrideInBytes = sizeof(LineVertex);
}

///----------------------------------------------------
/// 座標変換行列バッファ生成
///----------------------------------------------------
void Line::CreateTransformationMatrixBuffer() {
    // 定数バッファのサイズを 256 バイトの倍数に設定
    size_t bufferSize = (sizeof(TransformationMatrix) + 255) & ~255;
    transfomationMatrixBuffer_ = DirectXCommon::GetInstance()->CreateBufferResource(bufferSize);
    // TransformationMatrixの初期化
    TransformationMatrix transformationMatrix = {};
    // 書き込むためのアドレスを取得
    transfomationMatrixBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
    // 単位行列を書き込む
    transformationMatrix.WVP = MakeIdentity4x4();
    *transformationMatrixData_ = transformationMatrix;
}
