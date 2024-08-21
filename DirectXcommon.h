#pragma once


#include<d3d12.h>
#include<dxgi1_6.h>
#include<wrl.h>
#include <format>

#include"Logger.h"
#include"StringUtility.h"

#include"WinApp.h"
class DirectXCommon
{
public:
	void Initialize(WinApp* winApp);
	void Update();

	//デバイスの初期化
	void Device_Initialize();
	//コマンド関連の初期化
	void Command_Initialize();
	//スワップチェーンの生成
	void SwapChain_Create();
	//深度バッファの生成
	void DepthBuffer_Create();
	//各種ディスクリプタヒープの生成
	void DescriptorHeap_Create();
	////レンダーターゲットビューの初期化
	//void RTV_Initialize();
	////深度ステンシルビューの初期化
	//void DSV_Initialize();
	////フェンスの生成
	//void Fence_Create();
	////ビューポート矩形の初期化
	//void viewport_Initialize();
	////シザリング矩形の初期化
	//void scissor_Initialize();
	////DXCコンパイラの生成
	//void dxcCompiler_Create();
	////ImGuiの初期化
	//void ImGui_Initialize();


	//DescriptorHeapのさくせいかんすう
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> CreateDescriptorHeap(
		Microsoft::WRL::ComPtr <ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);


	//Microsoft::WRL::ComPtr <ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr <ID3D12Device> device, int32_t width, int32_t height);


	//
	//∧__∧
	//(｀Д´ ）
	//(っ▄︻▇〓┳═getter
	///    )
	//(/ ￣∪
	// 
	//Device
	HRESULT GetHr()const { return hr; }
	Microsoft::WRL::ComPtr <IDXGIFactory7> GetDxgiFactory()const { return dxgiFactory; }
	Microsoft::WRL::ComPtr <ID3D12Device> GetDevice()const { return device; }

	//Command
	Microsoft::WRL::ComPtr <ID3D12CommandAllocator> GetCommandAllocator()const { return commandAllocator; }
	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> GetCommandList()const { return commandList; }
	Microsoft::WRL::ComPtr <ID3D12CommandQueue> GetCommandQueue()const { return commandQueue; }

	//swapChain
	//swapChain
	Microsoft::WRL::ComPtr <IDXGISwapChain4> GetSwapChain()const { return swapChain; }
	DXGI_SWAP_CHAIN_DESC1 GetSwapChainDesc()const { return swapChainDesc; }


	//CPUのDescriptorHandleを取得する関数
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);

	//GPUのDescriptorHandleを取得する関数
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);


	//SRVの指定番号のCPUディスクリプタハンドルを取得する
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);
	//SRVの指定番号のGPUディスクリプタハンドルを取得する
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);


	//RTVの指定番号のCPUディスクリプタハンドルを取得する
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVCPUDescriptorHandle(uint32_t index);
	//RTVの指定番号のGPUディスクリプタハンドルを取得する
	D3D12_GPU_DESCRIPTOR_HANDLE GetRTVGPUDescriptorHandle(uint32_t index);


	//DSVの指定番号のCPUディスクリプタハンドルを取得する
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUDescriptorHandle(uint32_t index);
	//DSVの指定番号のGPUディスクリプタハンドルを取得する
	D3D12_GPU_DESCRIPTOR_HANDLE GetDSVGPUDescriptorHandle(uint32_t index);


private:

	WinApp* winApp_ = nullptr;

	//DXGIファクトリーの設置
	Microsoft::WRL::ComPtr <IDXGIFactory7> dxgiFactory = nullptr;

	//HRESULTはWindow系のエラーコードであり、
	//関数が成功したかどうかSUCCEEDEDマクロで判断出来る
	HRESULT hr;

	Microsoft::WRL::ComPtr <ID3D12Device> device = nullptr;

	//Command
	Microsoft::WRL::ComPtr <ID3D12CommandAllocator> commandAllocator = nullptr;
	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList = nullptr;
	Microsoft::WRL::ComPtr <ID3D12CommandQueue> commandQueue = nullptr;

	//swapChain
	Microsoft::WRL::ComPtr <IDXGISwapChain4> swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
};

