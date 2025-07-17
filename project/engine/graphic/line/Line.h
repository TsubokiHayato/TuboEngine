#pragma once
#include "EulerTransform.h"
#include "TransformationMatrix.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <wrl/client.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")

///----------------------------------------------------
/// Line描画用頂点構造体
///----------------------------------------------------
struct LineVertex {
    Vector3 position;
    Vector4 color;
};

class Camera;
class LineCommon;

///----------------------------------------------------
/// Line描画を管理するクラス
///----------------------------------------------------
class Line {
public:
    ///<summary>初期化処理</summary>
    void Initialize(LineCommon* lineCommon);
    ///<summary>更新処理</summary>
    void Update();
    ///<summary>描画処理</summary>
    void Draw();
    ///<summary>頂点情報クリア</summary>
    void ClearLines();
    ///<summary>ライン描画用頂点追加</summary>
    void DrawLine(const Vector3& start, const Vector3& end, const Vector4& color);

private:
    ///<summary>頂点バッファ生成</summary>
    void CreateVertexBuffer();
    ///<summary>座標変換行列バッファ生成</summary>
    void CreateTransformationMatrixBuffer();

public:
    ///<summary>Transformの設定</summary>
    void SetTransform(const EulerTransform& transform) { transform_ = transform; }
    ///<summary>Transformの取得</summary>
    EulerTransform GetTransform() const { return transform_; }
    ///<summary>スケールの設定</summary>
    void SetScale(const Vector3& scale) { transform_.scale = scale; }
    ///<summary>スケールの取得</summary>
    const Vector3& GetScale() const { return transform_.scale; }
    ///<summary>回転の設定</summary>
    void SetRotation(const Vector3& rotate) { transform_.rotate = rotate; }
    ///<summary>回転の取得</summary>
    const Vector3& GetRotation() const { return transform_.rotate; }
    ///<summary>位置の設定</summary>
    void SetPosition(const Vector3& translate) { transform_.translate = translate; }
    ///<summary>位置の取得</summary>
    const Vector3& GetPosition() const { return transform_.translate; }
    ///<summary>カメラの設定</summary>
    void SetCamera(Camera* camera) { this->camera_ = camera; }

private:
    ///----------------------------------------------------
    /// Line描画の共通処理管理
    ///----------------------------------------------------
    LineCommon* lineCommon_ = nullptr;

    ///----------------------------------------------------
    /// ライン描画用頂点配列
    ///----------------------------------------------------
    std::vector<LineVertex> vertices_;

    ///----------------------------------------------------
    /// 頂点バッファリソース
    ///----------------------------------------------------
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;

    ///----------------------------------------------------
    /// 頂点バッファビュー
    ///----------------------------------------------------
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = {};

    ///----------------------------------------------------
    /// 座標変換行列バッファリソース
    ///----------------------------------------------------
    Microsoft::WRL::ComPtr<ID3D12Resource> transfomationMatrixBuffer_;

    ///----------------------------------------------------
    /// 座標変換行列データポインタ
    ///----------------------------------------------------
    TransformationMatrix* transformationMatrixData_ = nullptr;

    ///----------------------------------------------------
    /// 座標変換情報
    ///----------------------------------------------------
    EulerTransform transform_ = {};

    ///----------------------------------------------------
    /// カメラ
    ///----------------------------------------------------
    Camera* camera_ = nullptr;
};
