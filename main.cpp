

#pragma comment(lib,"dxguid.lib")

#pragma comment(lib,"dxcompiler.lib")

#include"MT_Matrix.h"


#include"vector"


#include <fstream>
#include<sstream>




#include "Input.h"

#include"DirectXcommon.h"
#include"D3DResourceLeakChecker.h"

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

	//D3DResourceLeakChecker leakChecker;

	DirectXCommon* dxCommon = nullptr;
	dxCommon = new DirectXCommon();

	dxCommon->Initialize(winApp);

	HRESULT hr = dxCommon->GetHr();
	Microsoft::WRL::ComPtr <ID3D12Device> device = dxCommon->GetDevice();
	Microsoft::WRL::ComPtr <IDXGIFactory7> dxgiFactory = dxCommon->GetDxgiFactory();

	Microsoft::WRL::ComPtr <ID3D12CommandAllocator> commandAllocator = dxCommon->GetCommandAllocator();
	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList = dxCommon->GetCommandList();
	Microsoft::WRL::ComPtr <ID3D12CommandQueue> commandQueue = dxCommon->GetCommandQueue();

	//swapChain
	Microsoft::WRL::ComPtr <IDXGISwapChain4> swapChain = dxCommon->GetSwapChain();
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = dxCommon->GetSwapChainDesc();


	uint32_t descriptorSizeSRV = dxCommon->GetDescriptorSizeSRV();
	uint32_t descriptorSizeRTV = dxCommon->GetDescriptorSizeRTV();
	uint32_t descriptorSizeDSV = dxCommon->GetDescriptorSizeDSV();


	//RTVディスクイリプタヒープの生成
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> rtvDescriptorHeap = dxCommon->GetRtvDescriptorHeap();

	//SRVディスクイリプタヒープの生成
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> srvDescriptorHeap = dxCommon->GetSrvDescriptorHeap();
	//DSVディスクイリプタヒープの生成
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> dsvDescriptorHeap = dxCommon->GetDsvDescriptorHeap();


	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = dxCommon->GetRtvDesc();

	Microsoft::WRL::ComPtr<ID3D12Fence> fence = dxCommon->GetFence();
	uint64_t fenceValue = dxCommon->GetFenceValue();
	HANDLE fenceEvent = dxCommon->GetFenceEvent();

	//ビューポート
	D3D12_VIEWPORT viewport = dxCommon->GetViewport();
	//シザー矩形
	D3D12_RECT scissorRect = dxCommon->GetScissorRect();









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
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
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
	Microsoft::WRL::ComPtr <IDxcBlob> vertexShaderBlob = dxCommon->CompileShader(L"Object3d.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlob = dxCommon->CompileShader(L"Object3d.PS.hlsl",
		L"ps_6_0");
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
		dxCommon->CreateBufferResource(sizeof(Material));
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
		dxCommon->CreateBufferResource(sizeof(Material));
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
		dxCommon->CreateBufferResource(sizeof(DirectionalLight));
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
		dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* wvpData = nullptr;
	//書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	//単位行列を書き込んでいく
	wvpData->WVP = MakeIdentity4x4();
	wvpData->World = MakeIdentity4x4();


#pragma endregion






#pragma region SpriteResource

	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResourceSprite = dxCommon->CreateBufferResource(sizeof(VertexData) * 6);

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


	Microsoft::WRL::ComPtr <ID3D12Resource> transformationMatrixResourceSprite = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
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
		dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);


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
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
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
	DirectX::ScratchImage mipImages = dxCommon->LoadTexture("Resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr <ID3D12Resource> textureResource = dxCommon->CreateTextureResource(metadata);
 Microsoft::WRL::ComPtr< ID3D12Resource> intermediateResource=	dxCommon->UploadTextureData(textureResource.Get(), mipImages);

	//2枚目のTextureを読んで転送する
	DirectX::ScratchImage mipImages2 = dxCommon->LoadTexture(modelData.material.textureFilePath);
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr <ID3D12Resource> textureResource2 = dxCommon->CreateTextureResource(metadata2);
	Microsoft::WRL::ComPtr< ID3D12Resource> intermediateResource2=dxCommon->UploadTextureData(textureResource2.Get(), mipImages2);

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
		/*-------------------
		 Windowsメッセージ処理
		-------------------*/
		if (winApp->ProcessMessage()) {
			break;
		}

		//ウィンドウにメッセージが来てたら最優先で処理させる

		/*-------------------
			 入力の更新
		-------------------*/

		/*-------
		  ImGui
		-------*/
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


		ImGui::ShowDemoWindow();
		ImGui::Render();



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


		/*-------------------
		　　　シーンの更新
	　　-------------------*/
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


		/*-----
		描画処理
		------*/


		/*-------------------
		　　DirectX描画開始
		　-------------------*/
		dxCommon->PreDraw();

		/*-------------------
		　　シーンの描画
	　　-------------------*/

#pragma region Command//Triangle

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
		/*-------------------
		　　DirectX描画終了
	  　　-------------------*/
		dxCommon->PostDraw();

	}

#pragma region AllRelease



	//リソースリークチェック
	delete input;

	winApp->Finalize();

	delete winApp;
	winApp = nullptr;

	CloseHandle(fenceEvent);

	delete dxCommon;







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
