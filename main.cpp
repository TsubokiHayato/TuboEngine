

#include<dxgidebug.h>
#pragma comment(lib,"dxguid.lib")

#include<dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")

#include"MT_Matrix.h"

#include"externals/imgui/imgui.h"
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"


#include "externals/DirectXTex/d3dx12.h"
#include "externals/DirectXTex/DirectXTex.h"
#include"vector"


#include <fstream>
#include<sstream>




#include "Input.h"
#include"WinApp.h"
#include"DirectXCommon.h"


# define PI 3.14159265359f

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


struct Vector2
{
	float x;
	float y;

};

struct Vector4
{
	float x;
	float y;
	float z;
	float w;
};

struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

struct VertexData {
	Vector4 pos;
	Vector2 texcoord;
	Vector3 normal;
};

struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

struct  TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

struct MaterialData {
	std::string textureFilePath;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

struct D3DResourceLeakChecker
{
#pragma region ReportLiveObjects
	~D3DResourceLeakChecker() {
		Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
			debug->Release();
		}
	}
#pragma endregion

};


//DescriptorHeapのさくせいかんすう
Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> CreateDescriptorHeap(
	Microsoft::WRL::ComPtr <ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {

	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));

	return descriptorHeap;
}



//Resource作成の関数
Microsoft::WRL::ComPtr <ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr <ID3D12Device> device, size_t sizeInBytes) {


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

//コンパイラーシェーダー関数	
IDxcBlob* CompileShader(
	//CompilerするShaderファイルへのパス
	const std::wstring& filePath,
	//Compilerに使用するProfile
	const wchar_t* profile,
	//初期化で生成したものを３つ
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler) {


	/*---------------
	1 mslファイルを読む
	---------------*/


	//これからシェーダをコンパイルする旨をログに出す
	Log(ConvertString(std::format(L"Begin CompileShader,path:{},profile:{}\n", filePath, profile)));

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
		Log(shaderError->GetStringPointer());
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
	Log(ConvertString(std::format(L"Compile Succeeded,path:{},profile:{}\n", filePath, profile)));

	//もう使わないリソースを解放
	shaderSource->Release();
	shaderResult->Release();
	//実行用のバイナリを解放
	return shaderBlob;

}


//LoadTexture関数
DirectX::ScratchImage LoadTexture(const std::string& filePath) {

	//テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW =ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミップマップの作成
	DirectX::ScratchImage mipImages;
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	//ミップマップ付きのデータを返す
	return mipImages;
}

Microsoft::WRL::ComPtr <ID3D12Resource> CreateTextureResource(Microsoft::WRL::ComPtr <ID3D12Device> device, const DirectX::TexMetadata& metadata) {

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
	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;//細かい設定
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//WriteBackポリシーでCPUアクセス可能
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//プロセッサの近くに配置

	/*----------
	Resourceの生成
	-----------*/

	Microsoft::WRL::ComPtr <ID3D12Resource> resource = nullptr;

	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,//Heapの設定
		D3D12_HEAP_FLAG_NONE,//Heapの特殊な設定。特になし
		&resourceDesc,//Resourceの設定
		D3D12_RESOURCE_STATE_GENERIC_READ,//初回のResourceState。Textureは基本読むだけ
		nullptr,//Clear最適値。使わないのでnullptr
		IID_PPV_ARGS(&resource));//作成するResorceポインタへのポインタ

	assert(SUCCEEDED(hr));

	return resource;

}

void UploadTextureData(Microsoft::WRL::ComPtr <ID3D12Resource> texture, const DirectX::ScratchImage& mipImages) {

	//Meta情報を取得
	const DirectX::TexMetadata& metaData = mipImages.GetMetadata();
	//全MipMapについて
	for (size_t mipLevel = 0; mipLevel < metaData.mipLevels; ++mipLevel) {

		//MipLevelを指定してImageを取得
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
		//Textureに転送
		HRESULT hr = texture->WriteToSubresource(
			UINT(mipLevel),
			nullptr,//全領域へコピ
			img->pixels,//元データアドレス
			UINT(img->rowPitch),//1ラインサイズ
			UINT(img->slicePitch)//1枚サイズ
		);
		assert(SUCCEEDED(hr));
	}
}


Microsoft::WRL::ComPtr <ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr <ID3D12Device> device, int32_t width, int32_t height) {

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
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//VRAM上に作る

	//深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;//1.0f(最大値)でクリア
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//フォーマット。Resourceと合わせる

	//Resourceの生成
	Microsoft::WRL::ComPtr <ID3D12Resource> resouce = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&resouce));
	assert(SUCCEEDED(hr));

	return resouce;

}

//CPUのDescriptorHandleを取得する関数
D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index) {

	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

//GPUのDescriptorHandleを取得する関数
D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index) {

	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}




MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filePath) {

	/*------------------------
	1 : 中で必要になる変数の宣言
	------------------------*/

	MaterialData materialData;
	std::string line;


	/*------------------------
	2 : ファイルを開く
	------------------------*/

	std::ifstream file(directoryPath + "/" + filePath);
	assert(file.is_open());

	/*---------------------------------------------
	3 : 実際にファイルを読み、MaterialDataを構築していく
	---------------------------------------------*/

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;

			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}


	/*------------------------
	4 : MaterialDataを返す
	------------------------*/

	return materialData;

}



//オブジェクト読み込む関数

ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename) {

	/*-------------
	1 : OBJファイル
	--------------*/
	ModelData modelData;//構築する
	std::vector<Vector4> positions;//位置
	std::vector<Vector3> normals;//法線
	std::vector<Vector2> texcoords;//テクスチャ座標
	std::string line;//ファイルから読んだ1行を格納するもの


	/*----------------------
	2 : OBJファイルを読み込む
	----------------------*/
	std::ifstream file(directoryPath + "/" + filename); //ファイルを読み込む
	assert(file.is_open());//開けなかったら止める



	/*-----------------------------
	3 : ファイルを読み、ModelDataを構築
	-------------------------------*/

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;//先頭の識別子を読む


		//identifierに応じた処理


		/*------------------
			頂点情報を読む
		------------------*/
		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}

		/*---------------
			三角形を作る
		----------------*/
		else if (identifier == "f") {


			VertexData triangle[3];

			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;

				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');
					elementIndices[element] = std::stoi(index);
				}

				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];

				position.x *= -1.0f;
				normal.x *= -1.0f;
				texcoord.y = 1.0f - texcoord.y;


				/*VertexData vertex = { position,texcoord,normal };
				modelData.vertices.push_back(vertex);*/

				triangle[faceVertex] = { position,texcoord,normal };
			}

			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib") {
			//materialTemplateLibraryの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}


	return modelData;

}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	WinApp* winApp = nullptr;
	winApp = new WinApp();

	winApp->Initialize();

	D3DResourceLeakChecker leakChecker;

	DirectXCommon* dxCommon = nullptr;
	dxCommon = new DirectXCommon();
	dxCommon->Initialize();

	HRESULT hr = dxCommon->GetHr();
	Microsoft::WRL::ComPtr <ID3D12Device> device = dxCommon->GetDevice();
	Microsoft::WRL::ComPtr <IDXGIFactory7> dxgiFactory = dxCommon->GetDxgiFactory();

#pragma region commandQueue
	//コマンドキューを生成する
	Microsoft::WRL::ComPtr <ID3D12CommandQueue> commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{ };
	hr = device->CreateCommandQueue(&commandQueueDesc,
		IID_PPV_ARGS(&commandQueue));
	//コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region commandList
	//コマンドアロケータを生成する
	Microsoft::WRL::ComPtr <ID3D12CommandAllocator> commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));

	//コマンドアロケータの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//コマンドリスト生成する
	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr,
		IID_PPV_ARGS(&commandList));

	//コマンドリストの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region SwapChain

	//スワップチェーンを生成する
	Microsoft::WRL::ComPtr <IDXGISwapChain4> swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
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

#pragma region DescriptorHeap

	//RTVディスクイリプタヒープの生成
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);

	//SRVディスクイリプタヒープの生成
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

	//DSVディスクイリプタヒープの生成
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> dsvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
#pragma endregion


#pragma region DescriptorSize


	const uint32_t descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

#pragma endregion

#pragma region SwapChangeからResourceを引っ張ってくる

	Microsoft::WRL::ComPtr <ID3D12Resource> swapChainResources[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));

	//うまく取得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));

#pragma endregion


#pragma region RTV
	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2dテクスチャとして書き込む

	//ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = GetCPUDescriptorHandle(rtvDescriptorHeap, descriptorSizeRTV, 0);

	//RTVを2つ作るのでディスクリプタを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

	//まず一つ目は最初の所につくる。作る場所をこちらで指定してあげる必要がある
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);

	//2つ目のディスクリプタハンドルをえる(自力で)
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//2つ目を作る
	device->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);

#pragma endregion



#pragma region DSV
	Microsoft::WRL::ComPtr <ID3D12Resource> depthStencilResource = CreateDepthStencilTextureResource(device, winApp->kClientWidth, winApp->kClientHeight);

	//DSVの設定

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	//
	device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, GetCPUDescriptorHandle(dsvDescriptorHeap, descriptorSizeDSV, 0));


#pragma endregion




#pragma region Fence&Event
	//
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	//
	HANDLE fenceEvent = CreateEvent(NULL, false, false, NULL);
	assert(fenceEvent != nullptr);

#pragma endregion



#pragma region DXC_Initialization//
	//dxcCompilerを初期化
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));


	IDxcIncludeHandler* includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));


#pragma endregion

#pragma region PSO(Pipeline_State_Object)//
	/*------------
	RootSignature
	------------*/

	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//DescriptorRange作成
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;//0から始まる
	descriptorRange[0].NumDescriptors = 1;//数は1
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVをつかう
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//Offsetを自動計算


	//RootParameter作成。02_01追加//02_03更新
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderでつかう
	rootParameters[0].Descriptor.ShaderRegister = 0;//レジスタ番号0とバインド

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//VertexShaderでつかう
	rootParameters[1].Descriptor.ShaderRegister = 0;//レジスタ番号0とバインド

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DESCRIPTOR_TABLEを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderでつかう
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderでつかう
	rootParameters[3].Descriptor.ShaderRegister = 1;//レジスタ番号1とバインド

	descriptionRootSignature.pParameters = rootParameters;//ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);//配列の長さ


#pragma region Sampler_03_00//

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイアニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない	
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMipMapをつかう
	staticSamplers[0].ShaderRegister = 0;//レジスタ番号0をつかう
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderでつかう
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);


#pragma endregion



	//シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr <ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr <ID3DBlob> errorBlob = nullptr;

	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリを元に作成
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	/*------------
	  InputLayOut
	------------*/
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;


	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	/*------------
	  BlendState
	------------*/



	D3D12_BLEND_DESC blendDesc{};
	//すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA; // provided code
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;


	/*------------------
	  RasterizerState
	------------------*/

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	/*-------------------
	  Vertex&Pixel_Shader
	-------------------*/

	//Shaderをコンパイルする
	Microsoft::WRL::ComPtr <IDxcBlob> vertexShaderBlob = CompileShader(L"Object3d.VS.hlsl",
		L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlob = CompileShader(L"Object3d.PS.hlsl",
		L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(pixelShaderBlob != nullptr);


	/*---------------
	DepthStencilDescの設定
	-------------------*/

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depthの機能を有効化
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;


	/*------------------
	 　 PSOを生成する
	------------------*/
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicPipelineStateDesc{};

	graphicPipelineStateDesc.pRootSignature = rootSignature.Get();//RootSignature
	graphicPipelineStateDesc.InputLayout = inputLayoutDesc;//InputLayOut

	graphicPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };//vertexShader

	graphicPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };//pixcelShader

	graphicPipelineStateDesc.BlendState = blendDesc;//BlendState
	graphicPipelineStateDesc.RasterizerState = rasterizerDesc;//RasterizerState

	//書き込むRTVの情報
	graphicPipelineStateDesc.NumRenderTargets = 1;
	graphicPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	//利用するトポロジ(形状)のタイプ。三角形
	graphicPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	//どのように画面に書き込むのかの設定
	graphicPipelineStateDesc.SampleDesc.Count = 1;
	graphicPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;


	//DepthStencil
	graphicPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//実際に生成
	Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipeLineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipeLineState));
	assert(SUCCEEDED(hr));

#pragma endregion


#pragma region Material_Resource_02_01
	//マテリアル用のリソースを作る。今回はColor1つ分のサイズを用意する
	Microsoft::WRL::ComPtr <ID3D12Resource> materialResource =
		CreateBufferResource(device, sizeof(Material));
	//マテリアルにデータを書き込む
	Material* materialData = nullptr;
	//書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//今回は赤を書き込んでみる
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = true;
	materialData->uvTransform = MakeIdentity4x4();

#pragma endregion


#pragma region Material_Resource_Sprite
	//マテリアル用のリソースを作る。今回はColor1つ分のサイズを用意する
	Microsoft::WRL::ComPtr <ID3D12Resource> materialResourceSprite =
		CreateBufferResource(device, sizeof(Material));
	//マテリアルにデータを書き込む
	Material* materialDataSprite = nullptr;
	//書き込むためのアドレスを取得
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	//今回は赤を書き込んでみる
	materialDataSprite->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialDataSprite->enableLighting = false;
	materialDataSprite->uvTransform = MakeIdentity4x4();

#pragma endregion

	Transform uvTransFormSprite{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};


#pragma region DirectionalLightData
	//平行光源用用のリソースを作る。今回はColor1つ分のサイズを用意する
	Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResource =
		CreateBufferResource(device, sizeof(DirectionalLight));
	//平行光源用にデータを書き込む
	DirectionalLight* directionalLightData = nullptr;
	//書き込むためのアドレスを取得
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));

	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.5f,-0.5f,0.0f };
	directionalLightData->intensity = 1.0f;



#pragma endregion



#pragma region TransformMatrixResource_02_02

	//WVP用のリソースを作る
	Microsoft::WRL::ComPtr <ID3D12Resource> wvpResource =
		CreateBufferResource(device, sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* wvpData = nullptr;
	//書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	//単位行列を書き込んでいく
	wvpData->WVP = MakeIdentity4x4();
	wvpData->World = MakeIdentity4x4();


#pragma endregion






#pragma region SpriteResource

	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResourceSprite = CreateBufferResource(device, sizeof(VertexData) * 6);

	//頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	//リソースの先頭のアドレスから使う
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
	//1頂点あたりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	VertexData* vertexDataSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));

	//三角形を2つ使って、四角形の作る
	/*---------------
		B-------D		1枚目 : ABCの三角形
		|		|		2枚目 : BDCの三角形
		|		|
		A-------C
	----------------*/
	//A
	vertexDataSprite[0].pos = { 0.0f,360.0f,0.0f,1.0f };
	vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	vertexDataSprite[0].normal = { 0.0f,0.0f,1.0f };

	//B

	vertexDataSprite[1].pos = { 0.0f,0.0f,0.0f,1.0f };
	vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	vertexDataSprite[1].normal = { 0.0f,0.0f,1.0f };

	//C

	vertexDataSprite[2].pos = { 640.0f,360.0f,0.0f,1.0f };
	vertexDataSprite[2].texcoord = { 1.0f,1.0f };
	vertexDataSprite[2].normal = { 0.0f,0.0f,1.0f };


	//D
	vertexDataSprite[3].pos = { 640.0f,0.0f,0.0f,1.0f };
	vertexDataSprite[3].texcoord = { 1.0f,0.0f };
	vertexDataSprite[3].normal = { 0.0f,0.0f,1.0f };


	Microsoft::WRL::ComPtr <ID3D12Resource> transformationMatrixResourceSprite = CreateBufferResource(device, sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* transformationMatrixDataSprite = nullptr;

	//書き込むためのアドレスを取得
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	//単位行列を書き込んでおく
	transformationMatrixDataSprite->WVP = MakeIdentity4x4();
	transformationMatrixDataSprite->World = MakeIdentity4x4();

#pragma endregion


#pragma region indexResourceSprite

	//WVP用のリソースを作る
	Microsoft::WRL::ComPtr <ID3D12Resource> indexResourceSprite =
		CreateBufferResource(device, sizeof(uint32_t) * 6);


	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};

	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();

	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;

	uint32_t* indexDataSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));

	indexDataSprite[0] = 0;    indexDataSprite[1] = 1;   indexDataSprite[2] = 2;
	indexDataSprite[3] = 1;    indexDataSprite[4] = 3;   indexDataSprite[5] = 2;



#pragma endregion



#pragma region ModelData
	//モデルよみこみ
	ModelData modelData = LoadObjFile("resources", "axis.obj");
	//頂点リソースを作る
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource = CreateBufferResource(device, sizeof(VertexData) * modelData.vertices.size());
	//頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	//頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
#pragma endregion


#pragma region ShaderResorceView


	//Textureを読んで転送する
	DirectX::ScratchImage mipImages = LoadTexture("Resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr <ID3D12Resource> textureResource = CreateTextureResource(device, metadata);
	UploadTextureData(textureResource.Get(), mipImages);

	//2枚目のTextureを読んで転送する
	DirectX::ScratchImage mipImages2 = LoadTexture(modelData.material.textureFilePath);
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr <ID3D12Resource> textureResource2 = CreateTextureResource(device, metadata2);
	UploadTextureData(textureResource2.Get(), mipImages2);

	//metaDataを基にSRVの	設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	//metaDataを基にSRVの	設定(2枚目)
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);



	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 1);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 1);
	//先頭はImGuiを使っているのでその次をつかう
	textureSrvHandleCPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//SRVの生成
	device->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);



	//SRVを作成するDescriptorHeapの場所を決める(2枚目)
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 2);
	//先頭はImGuiを使っているのでその次をつかう(2枚目)
	textureSrvHandleCPU2.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU2.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//SRVの生成(2枚目)
	device->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);


#pragma endregion



#pragma region Viewport&Scissor//
	//ビューポート
	D3D12_VIEWPORT viewport{};
	//クライアント領域の領域のサイズと一緒にして画面全体に表示
	viewport.Width = (float)winApp->kClientWidth;
	viewport.Height = (float)winApp->kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	//シザー矩形
	D3D12_RECT scissorRect{};
	//基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect.left = 0;
	scissorRect.right = winApp->kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = winApp->kClientHeight;

#pragma endregion




#pragma region ImGui_Initialize

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp->GetHWND());
	ImGui_ImplDX12_Init(device.Get(),
		swapChainDesc.BufferCount,
		rtvDesc.Format,
		srvDescriptorHeap,
		GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 0),
		GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 0));

#pragma endregion



	Input* input = nullptr;

	input = new Input();
	input->Initialize(winApp);




	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Transform cameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-5.0f} };
	Matrix4x4 projectionMatrix = MakePerspectiveMatrix(0.45f, float(winApp->kClientWidth) / float(winApp->kClientHeight), 0.1f, 100.0f);

	Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };


	bool useMonsterBall = true;



	//ウィンドウの×ボタンんが押されるまでループ
	while (true) {
		if (winApp->ProcessMessage()) {
			break;
		}
		//ウィンドウにメッセージが来てたら最優先で処理させる



		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();


		ImGui::Text("Material");
		ImGui::Checkbox("useMonsterBall", &useMonsterBall);
		ImGui::DragFloat3("Translate", &transform.translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("Scale", &transform.scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("Rotate", &transform.rotate.x, 0.01f, -10.0f, 10.0f);


		ImGui::ColorEdit4("material.color", &materialData->color.x);
		ImGui::SliderFloat("intensity", &directionalLightData->intensity, 0.0f, 30.0f);
		ImGui::Text("Sprite");
		ImGui::DragFloat("UVTranslate", &uvTransFormSprite.translate.x, 0.01f, -1000.0f, 1000.0f);
		ImGui::DragFloat("UVScale", &uvTransFormSprite.scale.x, 0.01f, -10.0f, 10.0f);

		ImGui::SliderAngle("UVRotate", &uvTransFormSprite.rotate.z);

		/*--------
		ゲームの処理
		---------*/

		input->Update();
		if (input->PushKey(DIK_A)) {
			cameraTransform.rotate.y += 0.01f;
		}
		if (input->PushKey(DIK_D)) {
			cameraTransform.rotate.y -= 0.01f;
		}
		if (input->PushKey(DIK_SPACE) && input->TriggerKey(DIK_SPACE)) {
			cameraTransform.rotate.x -= 0.01f;
		}

		//transform.rotate.y += 0.03f;
		Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveMatrix(0.45f, float(winApp->kClientWidth) / float(winApp->kClientHeight), 0.1f, 100.0f);
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

		wvpData->WVP = worldViewProjectionMatrix;

		wvpData->World = worldMatrix;


		Matrix4x4 uvTransformMatrix = MakeAffineMatrix(uvTransFormSprite.scale, uvTransFormSprite.rotate, uvTransFormSprite.translate);

		materialDataSprite->uvTransform = uvTransformMatrix;

		Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
		Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
		Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(winApp->kClientWidth), float(winApp->kClientHeight), 0.0f, 100.0f);
		Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
		transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;
		transformationMatrixDataSprite->World = worldMatrixSprite;






		//ImGui::ShowDemoWindow();
		ImGui::Render();




		/*-----
		描画処理
		------*/

		//これから書き込むバックバッファのインデックスを取得
		UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

#pragma region TransitionBarrier

		//TransitionBarrierの設定
		D3D12_RESOURCE_BARRIER barrier{};
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


		//depthかきかえ
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle =
			GetCPUDescriptorHandle(dsvDescriptorHeap, descriptorSizeDSV, 0);
		commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);


		//指定した色で画面全体をクリアする
		float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };//RGBの値。青っぽい色
		commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

		//depthClear
		commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

#pragma endregion

		//描画用のDescriptorの設定
		ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap.Get() };
		commandList->SetDescriptorHeaps(1, descriptorHeaps);


#pragma region Command//Triangle

		commandList->RSSetViewports(1, &viewport);//viewPortを設定
		commandList->RSSetScissorRects(1, &scissorRect);//Scissorを設定
		//RootSignatureを設定。
		commandList->SetGraphicsRootSignature(rootSignature.Get());
		commandList->SetPipelineState(graphicsPipeLineState.Get());//PSOを設定
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);//VBVを設定
		//形状設定
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//マテリアルCBufferの場所を設定_02_01
		commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());


		//wvp用のCBufferの場所を設定_02_02
		commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());

		//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
		commandList->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);

		commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());


		//描画

		commandList->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);



		//Spriteの描画
		commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
		commandList->IASetIndexBuffer(&indexBufferViewSprite);

		commandList->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());

		//TransformationMatrixCBufferの設定
		commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());

		commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
		//描画
		commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

#pragma endregion

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());

#pragma region TransitionBarrier
		//画面に書く処理がすべて終わり、画面に映すので、状態を偏移
		// 今回はRenderTargetからPresentにする
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;

		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

		//TransitionBarrierを張る
		commandList->ResourceBarrier(1, &barrier);


		//コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
		hr = commandList->Close();
		assert(SUCCEEDED(hr));

#pragma endregion

#pragma region commandKick

		//GPUにコマンドリストの実行行わせる
		ID3D12CommandList* commandLists[] = { commandList.Get() };
		commandQueue->ExecuteCommandLists(1, commandLists);

		//GPUとOSに画面の交換を行うようにする
		swapChain->Present(1, 0);
#pragma endregion
#pragma region GPU-signal

		//Fenceの値を更新
		fenceValue++;
		//GPUがここまでたどり着いた時に、Fenceの値を指定した値に代入するようにSignalをおくる
		commandQueue->Signal(fence.Get(), fenceValue);


		if (fence->GetCompletedValue() < fenceValue) {
			//
			fence->SetEventOnCompletion(fenceValue, fenceEvent);
			//イベント待つ
			WaitForSingleObject(fenceEvent, INFINITE);
		}

		//次のフレーム用のコマンドリストを準備
		hr = commandAllocator->Reset();
		assert(SUCCEEDED(hr));
		hr = commandList->Reset(commandAllocator.Get(), nullptr);
		assert(SUCCEEDED(hr));

#pragma endregion



	}

#pragma region AllRelease



	//リソースリークチェック
	delete input;

	winApp->Finalize();

	delete winApp;
	winApp = nullptr;

	CloseHandle(fenceEvent);









	//ImGui
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	//警告時に止まる
	//infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

#pragma endregion




	return 0;

}
>>>>>>> a
