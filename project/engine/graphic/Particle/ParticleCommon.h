#pragma once
#pragma once
#include"WinApp.h"
#include"DirectXCommon.h"
#include"SrvManager.h"
class ParticlePSO;
class Camera;
class ParticleCommon
{

public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectX共通部分</param>
	void Initialize(SrvManager* srvManager);

	/// <summary>
	/// 共通描画設定
	/// </summary>
	void DrawSettingsCommon();

	

	void SetDefaultCamera(Camera* camera) { defaultCamera = camera; }
	Camera* GetDefaultCamera()const { return defaultCamera; }

	SrvManager* GetSrvManager() const { return srvManager_; }
private:

	ParticlePSO* pso = nullptr;//PSO
	Camera* defaultCamera = nullptr;//デフォルトカメラ

	SrvManager* srvManager_ = nullptr;//SRV共通部分

};

