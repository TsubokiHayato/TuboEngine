//#include "OffScreenRendering.h"
//#include <cassert>
//
//Microsoft::WRL::ComPtr<ID3D12Resource> OffScreenRendering::CreateRenderTargetResource(Microsoft::WRL::ComPtr<ID3D12Device>& device, int32_t width, int32_t height, DXGI_FORMAT format, const Vector4& clearColor){
//	// リソースの設定
//	D3D12_RESOURCE_DESC resourceDesc = {};
//	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2Dテクスチャ
//	resourceDesc.Width = static_cast<UINT64>(width);            // テクスチャの幅
//	resourceDesc.Height = static_cast<UINT>(height);            // テクスチャの高さ
//	resourceDesc.DepthOrArraySize = 1;                          // 奥行きまたは配列サイズ
//	resourceDesc.MipLevels = 1;                                 // ミップマップレベル
//	resourceDesc.Format = format;                               // テクスチャフォーマット
//	resourceDesc.SampleDesc.Count = 1;                          // サンプリング数（マルチサンプリングなし）
//	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;         // レイアウト（デフォルト）
//	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // レンダーターゲットとして使用
//
//	// クリア値の設定
//	D3D12_CLEAR_VALUE clearValue = {};
//	clearValue.Format = format;                                 // フォーマット
//	clearValue.Color[0] = clearColor.x;                         // クリアカラー (R)
//	clearValue.Color[1] = clearColor.y;                         // クリアカラー (G)
//	clearValue.Color[2] = clearColor.z;                         // クリアカラー (B)
//	clearValue.Color[3] = clearColor.w;                         // クリアカラー (A)
//
//	// ヒーププロパティの設定
//	D3D12_HEAP_PROPERTIES heapProperties = {};
//	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;              // デフォルトヒープ
//
//	// リソースの作成
//	Microsoft::WRL::ComPtr<ID3D12Resource> renderTargetResource;
//	HRESULT hr = device->CreateCommittedResource(
//		&heapProperties,                                       // ヒーププロパティ
//		D3D12_HEAP_FLAG_NONE,                                  // ヒープフラグ
//		&resourceDesc,                                         // リソース記述子
//		D3D12_RESOURCE_STATE_RENDER_TARGET,                   // 初期リソースステート
//		&clearValue,                                           // クリア値
//		IID_PPV_ARGS(&renderTargetResource)                   // 作成されたリソース
//	);
//
//	// 作成に失敗した場合はエラーを出力
//	if (FAILED(hr)) {
//		Logger::Log(std::format("Failed to create render target resource. HRESULT = {:#010x}\n", hr));
//		assert(SUCCEEDED(hr));
//	}
//
//	return renderTargetResource;
//}
//
//void OffScreenRendering::ClearRenderTargetPreDraw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, Microsoft::WRL::ComPtr<ID3D12Resource>& renderTarget) {
//
//
//	//RTVの作成
//	const Vector4 kRenderTargetClearValue = { 1.0f,0.0f,0.0f,1.0f }; // わかりやすいように赤色でクリア
//	auto renderTextureResource = CreateRenderTargetResource(device, winApp->kClientWidth, winApp->kClientHeight,
//		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTargetClearValue);
//	device->CreateRenderTargetView(renderTextureResource.Get(), nullptr, GetRTVCPUDescriptorHandle(0));
//
//
//	//SrVの作成
//	D3D12_SHADER_RESOURCE_VIEW_DESC renderTextureSRVDesc{};
//	renderTextureSRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
//	renderTextureSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
//	renderTextureSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//	renderTextureSRVDesc.Texture2D.MipLevels = 1;
//
//	//Srvの作成
//	device->CreateShaderResourceView(renderTextureResource.Get(), &renderTextureSRVDesc, GetSRVCPUDescriptorHandle(0));
//}
