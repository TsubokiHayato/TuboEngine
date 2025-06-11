#pragma once
#include <vector>
#include <memory>
#include "Camera.h"

class CameraManager
{
public:
    CameraManager();
    ~CameraManager();

    // カメラを追加
    void AddCamera(std::shared_ptr<Camera> camera);

    // アクティブカメラを設定
    void SetActiveCamera(size_t index);

    // アクティブカメラ取得
    std::shared_ptr<Camera> GetActiveCamera() const;

    // 全カメラ更新
    void UpdateAll();

    // カメラ数取得
    size_t GetCameraCount() const;

    // カメラ取得
    std::shared_ptr<Camera> GetCamera(size_t index) const;

private:
    std::vector<std::shared_ptr<Camera>> cameras_;
    size_t activeCameraIndex_;
};
