#pragma once
#include "Camera.h"
#include "LinePSO.h"
#include <memory>
#include <vector>

///----------------------------------------------------
/// Line描画の共通処理を管理するクラス
///----------------------------------------------------
class LineCommon {
public:
    ///<summary>初期化処理</summary>
    void Initialize();
    ///<summary>描画設定を行う</summary>
    void DrawSettingsCommon();

    ///<summary>デフォルトカメラの設定</summary>
    void SetDefaultCamera(Camera* camera) { defaultCamera = camera; }
    ///<summary>デフォルトカメラの取得</summary>
    Camera* GetDefaultCamera() const { return defaultCamera; }

private:
    ///----------------------------------------------------
    /// Line描画用パイプラインステートオブジェクト
    ///----------------------------------------------------
    std::unique_ptr<LinePSO> pso_;

    ///----------------------------------------------------
    /// デフォルトカメラ
    ///----------------------------------------------------
    Camera* defaultCamera = nullptr;
};