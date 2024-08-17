#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>
#include<wrl.h>

class DirectXcommon
{
public:
	void Initialize();
	void Update();


	//デバイスの初期化
	void Device_Initialize();
	//コマンド関連の初期化
	void Command_Initialize();
	//スワップチェーンの生成
	void SwapChain_Create();
private:
	//DirectXデバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	//DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
};

