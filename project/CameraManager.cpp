#include "CameraManager.h"

CameraManager::CameraManager()
    : activeCameraIndex_(0) {}

CameraManager::~CameraManager() = default;

void CameraManager::AddCamera(std::shared_ptr<Camera> camera) {
    cameras_.push_back(camera);
    if (cameras_.size() == 1) {
        activeCameraIndex_ = 0;
    }
}

void CameraManager::SetActiveCamera(size_t index) {
    if (index < cameras_.size()) {
        activeCameraIndex_ = index;
    }
}

std::shared_ptr<Camera> CameraManager::GetActiveCamera() const {
    if (cameras_.empty()) return nullptr;
    return cameras_[activeCameraIndex_];
}

void CameraManager::UpdateAll() {
    for (auto& camera : cameras_) {
        camera->Update();
    }
}

size_t CameraManager::GetCameraCount() const {
    return cameras_.size();
}

std::shared_ptr<Camera> CameraManager::GetCamera(size_t index) const {
    if (index < cameras_.size()) {
        return cameras_[index];
    }
    return nullptr;
}
