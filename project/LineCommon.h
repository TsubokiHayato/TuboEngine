#pragma once

#include "Camera.h"
#include "DirectXCommon.h"
class LineCommon {

public:
	void Initialize();
	void DrawSettingsCommon();

private:
	void CreateRootSignature();
	void CreateGraphicsPipeline();

public:
	void SetDefaultCamera(Camera* camera) { defaultCamera = camera; }
	Camera* GetDefaultCamera() const { return defaultCamera; }

private:
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState;

	Camera* defaultCamera = nullptr;
};
