#include "DirectXcommon.h"


#include<cassert>
#include <thread>
#include<SrvManager.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


using namespace Microsoft::WRL;
//
//DirectXCommon* DirectXCommon::GetInstance()
//{
//
//	if (instance == nullptr) {
//		instance = new DirectXCommon;
//	}
//	return instance;
//
//}

void DirectXCommon::Initialize(WinApp* winApp) {
	//NULL検出
	assert(winApp);
	//メンバ変数に記録
	this->winApp = std::make_unique<WinApp>(*winApp);

	//FPS固定初期化
	InitializeFixFPS();

	//デバイスの初期化
	Device_Initialize();
	//コマンド関連の初期化
	Command_Initialize();
	//スワップチェーンの生成
	SwapChain_Create();
	//深度バッファの生成
	DepthBuffer_Create(winApp->kClientWidth, winApp->kClientHeight);
	//各種ディスクリプタヒープの生成
	DescriptorHeap_Create();


	//レンダーターゲットの初期化
	RenderTargetInitialize();

	//レンダーターゲットビューの初期化
	RTV_Initialize();
	//深度ステンシルビューの初期化
	DSV_Initialize();
	//フェンスの生成
	Fence_Create();
	//ビューポート矩形の初期化
	Viewport_Initialize();
	//シザリング矩形の初期化
	Scissor_Initialize();
	//DXCコンパイラの生成
	dxcCompiler_Create();

	offScreenRenderingPSO = new OffScreenRenderingPSO();
	offScreenRenderingPSO->Initialize(this);


}


void DirectXCommon::Device_Initialize() {


#ifdef _DEBUG

	Microsoft::WRL::ComPtr <ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		//デバックレイヤを有効化する
		debugController->EnableDebugLayer();
		//さらにGPU側でもチェックを行えるようにする
		debugController->SetEnableGPUBasedValidation(true);

	}

#endif



#pragma region DXGIFactory
	/*DXGIFactoryの作成*/


	//HRESULTはWindow系のエラーコードであり、
	//関数が成功したかどうかSUCCEEDEDマクロで判断出来る
	hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));

	//初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、
	//どうにもできない場合が多いのでassertにしておく

	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region adapter
	//使用するアダプタ用の変数
	Microsoft::WRL::ComPtr <IDXGIAdapter4> useAdapter = nullptr;
	//一番いい並びのアダプタを頼む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
		DXGI_ERROR_NOT_FOUND; ++i) {
		//アダプタの情報取得
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));
		//ソフトウェアアダプタ出ないならば採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//採用したアダプタの情報をログに出力
			Logger::Log(StringUtility::ConvertString(std::format(L"USE Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		//ソフトウェアアダプタならば見なかったことにする
		useAdapter = nullptr;
	}
	//適切なアダプタが見つからなかったので起動できない
	assert(useAdapter != nullptr);

#pragma endregion

#pragma region D3D12Device


	//機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
	D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
	//高い順採用できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); i++) {
		//採用したアダプターでデバイスを生成
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
		//指定した機能レベルでデバイスが生成できたか確認
		if (SUCCEEDED(hr)) {
			//生成できたのでロ出力をしてループを抜ける
			Logger::Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}

	}
	//デバイスの生成がうまくいかなかったので起動できない
	assert(device != nullptr);
	//初期化完了のログを出す
	Logger::Log("Complete create D3D12Device!!!\n");

#pragma endregion 

#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		//やばいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);


		D3D12_MESSAGE_ID denyIds[] = {
			//window11でのDXGデバッグレイヤの相互作用バグによるエラーメッセージ
			//https://stackoverflow.com/questions/69805245/directx-12-application-crashing-window-11
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);
		//解放
		infoQueue->Release();

	}
#endif


}

void DirectXCommon::Command_Initialize() {


	//コマンドアロケータを生成する

	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));

	//コマンドアロケータの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));
#pragma region commandList


	//コマンドリスト生成する

	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr,
		IID_PPV_ARGS(&commandList));

	//コマンドリストの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region commandQueue
	//コマンドキューを生成する

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{ };
	hr = device->CreateCommandQueue(&commandQueueDesc,
		IID_PPV_ARGS(&commandQueue));
	//コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));
#pragma endregion

}

void DirectXCommon::SwapChain_Create() {

#pragma region SwapChain

	//スワップチェーンを生成する

	swapChainDesc.Width = winApp->kClientWidth;//画面の幅。ウィンドウのいクライアント領域を同じものにしておく
	swapChainDesc.Height = winApp->kClientHeight;//画面の高さ。ウィンドウのいクライアント領域を同じものにしておく
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
	swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//描画のターゲットとして利用する
	swapChainDesc.BufferCount = 2;//ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニタにうつしたら、中身を破棄

	//コマンドキュー、ウィンドウハンドル、せっていを渡して生成する
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winApp->GetHWND(), &swapChainDesc,
		nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));



#pragma endregion


#pragma region SwapChangeからResourceを引っ張ってくる


	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	swapChainResources[0]->SetName(L"SwapChainBuffer0");

	//うまく取得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	swapChainResources[1]->SetName(L"SwapChainBuffer1");
	assert(SUCCEEDED(hr));

#pragma endregion



}

void DirectXCommon::DepthBuffer_Create(int32_t width, int32_t height) {

	//生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;//Textureの幅
	resourceDesc.Height = height;//Textureの高さ
	resourceDesc.MipLevels = 1;//mipmapの数
	resourceDesc.DepthOrArraySize = 1;//奥行きor配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//DepthStencilとして利用可能なフォーマット
	resourceDesc.SampleDesc.Count = 1;//サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//二次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; //DepthStencilとして使う通知

	//利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;//細かい設定を行う
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;



	//深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;//1.0f(最大値)でクリア
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//フォーマット。Resourceと合わせる

	//Resourceの生成

	hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&depthStencilResource));

	if (FAILED(hr)) {
		Logger::Log(std::format("Failed to create depth stencil resource. HRESULT = {:#010x}\n", hr));
	}
	assert(SUCCEEDED(hr));



}

void DirectXCommon::DescriptorHeap_Create() {


#pragma region DescriptorSize


	descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

#pragma endregion

#pragma region DescriptorHeap


	//RTVディスクイリプタヒープの生成
	rtvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);

	

	//SRVディスクイリプタヒープの生成
	srvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, SrvManager::kMaxSRVCount, true);

	//DSVディスクイリプタヒープの生成
	dsvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
#pragma endregion


}

void DirectXCommon::RTV_Initialize() {



#pragma region RTV
	//RTVの設定

	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2dテクスチャとして書き込む

	//ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = GetCPUDescriptorHandle(rtvDescriptorHeap, descriptorSizeRTV, 0);

	//RTVを2つ作るのでディスクリプタを2つ用意


	//まず一つ目は最初の所につくる。作る場所をこちらで指定してあげる必要がある
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);

	//2つ目のディスクリプタハンドルをえる(自力で)
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//2つ目を作る
	device->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);

	// DescriptorHeap_Create で offscreenRtvDescriptorHeap を作成済み
	offscreenRtvHandle = offscreenRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	device->CreateRenderTargetView(renderTextureResource_.Get(), nullptr, offscreenRtvHandle);




#pragma endregion
}

void DirectXCommon::DSV_Initialize() {



#pragma region DSV

	//DSVの設定

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	//
	device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, GetCPUDescriptorHandle(dsvDescriptorHeap, descriptorSizeDSV, 0));


#pragma endregion

}

void DirectXCommon::Fence_Create() {

#pragma region Fence&Event

	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	fenceEvent = CreateEvent(NULL, false, false, NULL);
	assert(fenceEvent != nullptr);

#pragma endregion


}

void DirectXCommon::Viewport_Initialize() {


	//クライアント領域の領域のサイズと一緒にして画面全体に表示
	viewport.Width = (float)winApp->kClientWidth;
	viewport.Height = (float)winApp->kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

}



void DirectXCommon::Scissor_Initialize() {

	//基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect.left = 0;
	scissorRect.right = winApp->kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = winApp->kClientHeight;
}

void DirectXCommon::dxcCompiler_Create() {



	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));


}

//描画前処理
void DirectXCommon::PreDraw() {
	/*バックバッファの番号取得*/

	//これから書き込むバックバッファのインデックスを取得
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();



	/*リリースバリアで書き込み可能に変更*/


	//今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//バリアを張る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();
	//偏移前(現在)のResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	//偏移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);




	/* 描画先のRTVとDSVを指定する*/
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle =
		GetDSVCPUDescriptorHandle(0);
	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);


	/* 描画全体の色をクリア*/


	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };//RGBの値。青っぽい色
	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);


	/* 画面全体の深度をクリア*/

	//depthClear
	//commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


	/* SRV用のディスクリプタヒープを指定する*/
			//描画用のDescriptorの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap.Get() };
	commandList->SetDescriptorHeaps(1, descriptorHeaps);


	/* ビューポート領域の指定*/
	commandList->RSSetViewports(1, &viewport);//viewPortを設定


	/* シザー矩形の設定*/
	commandList->RSSetScissorRects(1, &scissorRect);//Scissorを設定

}




//描画後処理
void DirectXCommon::PostDraw() {
	/*バックバッファの番号取得*/
	UINT bbIndex = swapChain->GetCurrentBackBufferIndex();


	renderingBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	;
	renderingBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	/*リソースバリアで表示状態に変更*/

#pragma region TransitionBarrier

	  //画面に書く処理がすべて終わり、画面に映すので、状態を偏移
	  // 今回はRenderTargetからPresentにする
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;

	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	// Render
	commandList->ResourceBarrier(1, &renderingBarrier);

	//TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);




#pragma endregion

	/*グラフィックスコマンドをクローズ*/

	//コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
	hr = commandList->Close();
	assert(SUCCEEDED(hr));


	/*GPUコマンドの実行*/

	//GPUにコマンドリストの実行行わせる
	ID3D12CommandList* commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(1, commandLists);


	/*GPU画面の交換を通知*/


//GPUとOSに画面の交換を行うようにする
	swapChain->Present(1, 0);

	/*Fenceの値を更新*/

	fenceValue++;


	/*コマンドキューにシグナルを送る*/

	//GPUがここまでたどり着いた時に、Fenceの値を指定した値に代入するようにSignalをおくる
	commandQueue->Signal(fence.Get(), fenceValue);


	/*コマンド完了待ち*/

	if (fence->GetCompletedValue() < fenceValue) {
		//
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		//イベント待つ
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	//FPS固定更新
	UpdateFixFPS();

	/*コマンドアロケーターのリセット*/

	//次のフレーム用のコマンドリストを準備
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));


	/*コマンドリストのリセット*/

	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));


}

void DirectXCommon::CommandExecution() {

#pragma endregion

	/*グラフィックスコマンドをクローズ*/

	//コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
	hr = commandList->Close();
	assert(SUCCEEDED(hr));


	/*GPUコマンドの実行*/

	//GPUにコマンドリストの実行行わせる
	ID3D12CommandList* commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(1, commandLists);


	/*GPU画面の交換を通知*/


	/*Fenceの値を更新*/

	fenceValue++;


	/*コマンドキューにシグナルを送る*/

	//GPUがここまでたどり着いた時に、Fenceの値を指定した値に代入するようにSignalをおくる
	commandQueue->Signal(fence.Get(), fenceValue);


	/*コマンド完了待ち*/

	if (fence->GetCompletedValue() < fenceValue) {
		//
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		//イベント待つ
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	//FPS固定更新
	//UpdateFixFPS();

	/*コマンドアロケーターのリセット*/

	//次のフレーム用のコマンドリストを準備
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));


	/*コマンドリストのリセット*/

	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));


}




Microsoft::WRL::ComPtr<IDxcBlob> DirectXCommon::CompileShader(const std::wstring& filePath, const wchar_t* profile) {

	/*---------------
	1 mslファイルを読む
	---------------*/


	//これからシェーダをコンパイルする旨をログに出す
	Logger::Log(StringUtility::ConvertString(std::format(L"Begin CompileShader,path:{},profile:{}\n", filePath, profile)));

	//hlslファイルを読み込む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);

	//読めなかったら止める
	assert(SUCCEEDED(hr));

	//読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;//UTF-8文字コードであることを通知

	/*-----------
	2 Compileする
	-----------*/

	LPCWSTR arguments[] = {
		filePath.c_str(),//読み込んだファイル
		L"-E",L"main",//エントリーポイントの指定
		L"-T",profile,//shaderProfileの設定
		L"-Zi",L"-Qembed_debug",//デバッグ用の情報を埋め込む
		L"-Od",//最適化を外しておく
		L"-Zpr",//メモリレイアウトは行優先
	};

	//実際にShaderをコンパイルを設定する
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,
		arguments,
		_countof(arguments),
		includeHandler,
		IID_PPV_ARGS(&shaderResult)
	);

	assert(SUCCEEDED(hr));


	/*--------------------------
	3 警告・エラーがでてないか確認する
	--------------------------*/

	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Logger::Log(shaderError->GetStringPointer());
		assert(false);
	}


	/*--------------------------
	　4 Compile結果を受け取って返す
	--------------------------*/

	//コンパイル結果から実行用のバイナリ部分を取得
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));

	//成功したログを出す
	Logger::Log(StringUtility::ConvertString(std::format(L"Compile Succeeded,path:{},profile:{}\n", filePath, profile)));

	//もう使わないリソースを解放
	shaderSource->Release();
	shaderResult->Release();
	//実行用のバイナリを解放
	return shaderBlob;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateBufferResource(size_t sizeInBytes) {

	Microsoft::WRL::ComPtr <IDXGIFactory7>dxgiFactory = nullptr;

	//HRESULTはWindow系のエラーコードであり、
	//関数が成功したかどうかSUCCEEDEDマクロで判断出来る
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));


	//リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	//リソースの設定
	D3D12_RESOURCE_DESC ResourceDesc{};


	//バッファリソース。テクスチャの場合別の設定をする
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ResourceDesc.Width = (sizeInBytes);//リソースのファイル。

	//バッファの場合はこれらを1にする決まり
	ResourceDesc.Height = 1;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.SampleDesc.Count = 1;

	//バッファの場合はこれにする決まり
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//実際にリソースを作る
	Microsoft::WRL::ComPtr <ID3D12Resource> Resource = nullptr;
	hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&ResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&Resource));
	assert(SUCCEEDED(hr));

	return Resource;

}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateTextureResource(const DirectX::TexMetadata& metadata) {

	/*------------------------
	metaDataを基にResourceの設定
	------------------------*/

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);//Textureの幅
	resourceDesc.Height = UINT(metadata.height);//Textureの高さ
	resourceDesc.MipLevels = UINT(metadata.mipLevels);//mipmapの数
	resourceDesc.DepthOrArraySize = UINT(metadata.arraySize);//奥行き　or 配列Textureの配列数
	resourceDesc.Format = metadata.format;//TextureのFormat
	resourceDesc.SampleDesc.Count = 1;//1固定。サンプリングカウント
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);//Textureの次元数。普段使っているのは2次元


	/*--------------
	利用するHeapの設定
	--------------*/

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	//heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;//細かい設定
	//heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//WriteBackポリシーでCPUアクセス可能
	//heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//プロセッサの近くに配置

	/*----------
	Resourceの生成
	-----------*/

	Microsoft::WRL::ComPtr <ID3D12Resource> resource = nullptr;

	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,//Heapの設定
		D3D12_HEAP_FLAG_NONE,//Heapの特殊な設定。特になし
		&resourceDesc,//Resourceの設定
		D3D12_RESOURCE_STATE_COPY_DEST,//初回のResourceState。Textureは基本読むだけ
		nullptr,//Clear最適値。使わないのでnullptr
		IID_PPV_ARGS(&resource));//作成するResorceポインタへのポインタ

	assert(SUCCEEDED(hr));

	return resource;

}

[[nodiscard]]
Microsoft::WRL::ComPtr <ID3D12Resource> DirectXCommon::
UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages) {
	std::vector<D3D12_SUBRESOURCE_DATA> subResources;
	DirectX::PrepareUpload(device.Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subResources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subResources.size()));

	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(intermediateSize);
	UpdateSubresources(commandList.Get(), texture.Get(), intermediateResource.Get(), 0, 0, UINT(subResources.size()), subResources.data());

	//Textureへの転送後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);

	return intermediateResource;
}

DirectX::ScratchImage DirectXCommon::LoadTexture(const std::string& filePath) {
	//テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミップマップの作成
	DirectX::ScratchImage mipImages;
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	//ミップマップ付きのデータを返す
	return mipImages;
}

void DirectXCommon::InitializeFixFPS() {

	//現在時間を記録する
	reference_ = std::chrono::steady_clock::now();


}

void DirectXCommon::UpdateFixFPS() {

	//1/60秒ピッタリの時間
	const std::chrono::microseconds kMinTime(uint64_t(1000000.0f / 60.0f));
	//1/60秒よりわずかに短い時間
	const std::chrono::microseconds kMinCheckTime(uint64_t(1000000.0f / 65.0f));

	//現在時間を取得する
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	//前回記録からの経過時間を取得する
	std::chrono::microseconds elapsed =
		std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

	//1/60秒(よりわずかに短い時間)経っていない場合
	if (elapsed < kMinCheckTime) {
		//1/601秒経過するまで微小なスリープを繰り返す
		while (std::chrono::steady_clock::now() - reference_ < kMinTime) {
			//1マイクロ秒スリープ
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}

	//現在の時間を記録する
	reference_ = std::chrono::steady_clock::now();
}



void DirectXCommon::RenderTargetInitialize() {
	// RTVの作成
	renderTextureResource_ = CreateRenderTargetResource(
		device, winApp->kClientWidth, winApp->kClientHeight,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTargetClearValue
	);
	renderTextureResource_->SetName(L"RenderTargetResource");

	// オフスクリーン用（必要な数だけ。ここでは1つ）
	offscreenRtvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1, false);
	// オフスクリーン用RTVディスクリプタの取得
	offscreenRtvHandle = offscreenRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	device->CreateRenderTargetView(renderTextureResource_.Get(), nullptr, offscreenRtvHandle);


	// SRVの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC renderTextureSRVDesc{};
	renderTextureSRVDesc.Format = renderTextureResource_->GetDesc().Format;
	renderTextureSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	renderTextureSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	renderTextureSRVDesc.Texture2D.MipLevels = 1;

	// SRVの生成
	device->CreateShaderResourceView(renderTextureResource_.Get(), &renderTextureSRVDesc, srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateRenderTargetResource(Microsoft::WRL::ComPtr<ID3D12Device>& device, int32_t width, int32_t height, DXGI_FORMAT format, const Vector4& clearColor) {
	// リソースの設定
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	resourceDesc.Width = static_cast<UINT64>(width);            // テクスチャの幅
	resourceDesc.Height = static_cast<UINT>(height);            // テクスチャの高さ
	resourceDesc.DepthOrArraySize = 1;                          // 奥行きまたは配列サイズ
	resourceDesc.MipLevels = 1;                                 // ミップマップレベル
	resourceDesc.Format = format;                               // テクスチャフォーマット
	resourceDesc.SampleDesc.Count = 1;                          // サンプリング数（マルチサンプリングなし）
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;         // レイアウト（デフォルト）
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // レンダーターゲットとして使用

	// クリア値の設定
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = format;                                 // フォーマット
	clearValue.Color[0] = clearColor.x;                         // クリアカラー (R)
	clearValue.Color[1] = clearColor.y;                         // クリアカラー (G)
	clearValue.Color[2] = clearColor.z;                         // クリアカラー (B)
	clearValue.Color[3] = clearColor.w;                         // クリアカラー (A)

	// ヒーププロパティの設定
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;              // デフォルトヒープ

	// リソースの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTargetResource;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,                                       // ヒーププロパティ
		D3D12_HEAP_FLAG_NONE,                                  // ヒープフラグ
		&resourceDesc,                                         // リソース記述子
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		//D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,                   // 初期リソースステート
		&clearValue,                                           // クリア値
		IID_PPV_ARGS(&renderTargetResource)                   // 作成されたリソース
	);

	// 作成に失敗した場合はエラーを出力
	if (FAILED(hr)) {
		Logger::Log(std::format("Failed to create render target resource. HRESULT = {:#010x}\n", hr));
		assert(SUCCEEDED(hr));
	}

	return renderTargetResource;
}


void DirectXCommon::ClearRenderTargetPreDraw() {

	//ばりあ
	renderingBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	renderingBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	renderingBarrier.Transition.pResource = renderTextureResource_.Get();

	// RTV/DSVの設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetRTVCPUDescriptorHandle(0); // renderTextureResource用
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetDSVCPUDescriptorHandle(0);
	commandList->OMSetRenderTargets(1, &offscreenRtvHandle, FALSE, &dsvHandle);
	// クリア
	FLOAT clearColor[4] = { kRenderTargetClearValue.x, kRenderTargetClearValue.y, kRenderTargetClearValue.z, kRenderTargetClearValue.w };
	commandList->ClearRenderTargetView(offscreenRtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// ビューポート/シザー設定
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
}


void DirectXCommon::TransitionRenderTextureToShaderResource() {
	//if (renderTextureState == D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) return;

	
	renderingBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	renderingBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	renderingBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	commandList->ResourceBarrier(1, &renderingBarrier);
	//renderTextureState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
}




void DirectXCommon::DrawOffScreenPass() {



	// 4. SRV用ディスクリプタヒープをセット
	ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap.Get() };
	commandList->SetDescriptorHeaps(1, descriptorHeaps);

	// 5. PSO・ルートシグネチャ設定
	offScreenRenderingPSO->DrawSettingsCommon();
	// 6. SRV（オフスクリーンテクスチャ）をルートパラメータにセット
	// ※ルートシグネチャのSRVインデックスに合わせて変更（例: 0番なら0）
	commandList->SetGraphicsRootDescriptorTable(0, GetSRVGPUDescriptorHandle(0));

	// 7. 全画面三角形を描画
	commandList->DrawInstanced(3, 1, 0, 0); // 全画面三角形


}
void DirectXCommon::TransitionRenderTextureToRenderTarget() {
	//if (renderTextureState == D3D12_RESOURCE_STATE_RENDER_TARGET) return;

	//
	//
	//// 🔧 状態を更新！
	//renderTextureState = D3D12_RESOURCE_STATE_RENDER_TARGET;
}





Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {

	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));

	return descriptorHeap;
}




D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index) {

	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index) {

	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVCPUDescriptorHandle(uint32_t index) {
	return GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVGPUDescriptorHandle(uint32_t index) {
	return  GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, index);
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetRTVCPUDescriptorHandle(uint32_t index) {
	return GetCPUDescriptorHandle(rtvDescriptorHeap, descriptorSizeRTV, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetRTVGPUDescriptorHandle(uint32_t index) {
	return  GetGPUDescriptorHandle(rtvDescriptorHeap, descriptorSizeRTV, index);
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetDSVCPUDescriptorHandle(uint32_t index) {
	return GetCPUDescriptorHandle(dsvDescriptorHeap, descriptorSizeDSV, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetDSVGPUDescriptorHandle(uint32_t index) {
	return  GetGPUDescriptorHandle(dsvDescriptorHeap, descriptorSizeDSV, index);
}