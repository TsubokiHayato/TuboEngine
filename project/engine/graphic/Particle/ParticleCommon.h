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
	/// シングルトンインスタンス取得
	/// </summary>
	static ParticleCommon* GetInstance() {
		static ParticleCommon instance;
		return &instance;
	}

private:
	// コンストラクタ・デストラクタ・コピー禁止
	ParticleCommon() = default;
	~ParticleCommon() = default;
	ParticleCommon(const ParticleCommon&) = delete;
	ParticleCommon& operator=(const ParticleCommon&) = delete;

public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectX共通部分</param>
	void Initialize();

	/// <summary>
	/// 共通描画設定
	/// </summary>
	void DrawSettingsCommon();

	

	void SetDefaultCamera(Camera* camera) { defaultCamera = camera; }
	Camera* GetDefaultCamera()const { return defaultCamera; }

private:

	ParticlePSO* pso = nullptr;//PSO
	Camera* defaultCamera = nullptr;//デフォルトカメラ

};

