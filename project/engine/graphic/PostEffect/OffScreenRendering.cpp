#include "OffscreenRendering.h"
#include "DepthBasedOutlineEffect.h"
#include "DirectXCommon.h"
#include "DissolveEffect.h"
#include "GaussianBlurEffect.h"
#include "GrayScaleEffect.h"
#include "ImGuiManager.h"
#include "NoneEffect.h"
#include "OutlineEffect.h"
#include "RadialBlurEffect.h"
#include "SepiaEffect.h"
#include "SmoothingEffect.h"
#include "VignetteEffect.h"
#include "WinApp.h"
#include "randomEffect.h"

OffScreenRendering* OffScreenRendering::instance = nullptr;

/// <summary>
/// オフスクリーンレンダリングの初期化処理
/// 必要なリソースの生成やディスクリプタの設定、PSOの初期化を行います。
/// </summary>
void OffScreenRendering::Initialize() {

	// メンバ変数に記録

	device = DirectXCommon::GetInstance()->GetDevice();
	commandList = DirectXCommon::GetInstance()->GetCommandList();

	// RTVの作成
	renderTextureResource_ =
	    CreateRenderTargetResource(device, WinApp::GetInstance()->GetClientWidth(), WinApp::GetInstance()->GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTargetClearValue);
	renderTextureResource_->SetName(L"RenderTargetResource");

	// オフスクリーン用（必要な数だけ。ここでは1つ）
	offscreenRtvDescriptorHeap = DirectXCommon::GetInstance()->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1, false);
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
	device->CreateShaderResourceView(renderTextureResource_.Get(), &renderTextureSRVDesc, DirectXCommon::GetInstance()->GetSrvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart());

	///---------------------------------------------------------------------
	///					PostEffectManagerの初期化
	///---------------------------------------------------------------------

	// PostEffectの追加
	// postEffectManager.AddEffect(std::make_unique<NoneEffect>());              // 何もしないエフェクト
	// postEffectManager.AddEffect(std::make_unique<GrayScaleEffect>());         // グレースケールエフェクト
	// postEffectManager.AddEffect(std::make_unique<SepiaEffect>());             // セピアエフェクト
	// postEffectManager.AddEffect(std::make_unique<VignetteEffect>());          // ビネットエフェクト
	// postEffectManager.AddEffect(std::make_unique<SmoothingEffect>());         // スムージングエフェクト
	// postEffectManager.AddEffect(std::make_unique<GaussianBlurEffect>());      // ガウスぼかしエフェクト
	// postEffectManager.AddEffect(std::make_unique<RadialBlurEffect>());        // ラジアルブラーエフェクト
	// postEffectManager.AddEffect(std::make_unique<OutlineEffect>());           // アウトラインエフェクト
	postEffectManager.AddEffect(std::make_unique<DepthBasedOutlineEffect>()); // 深度ベースのアウトラインエフェクト
	// postEffectManager.AddEffect(std::make_unique<DissolveEffect>());          // ディゾルブエフェクト
	// postEffectManager.AddEffect(std::make_unique<randomEffect>());            // ランダムエフェクト

	// PostEffectManagerの初期化
	postEffectManager.InitializeAll();
}

void OffScreenRendering::Update() {

	// 　PostEffectManagerの更新
	postEffectManager.UpdateAll();
}

/// <summary>
/// 描画前の設定処理
/// レンダーターゲットや深度ステンシルのセット、クリア、ビューポート・シザーの設定を行います。
/// </summary>
void OffScreenRendering::PreDraw() {

//	TransitionDepthTo(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	// ばりあ
	renderingBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	renderingBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	renderingBarrier.Transition.pResource = renderTextureResource_.Get();

	 auto depthResource = DirectXCommon::GetInstance()->GetDepthStencliResouece();
	depthBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	depthBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	depthBarrier.Transition.pResource = depthResource.Get();

	// RTV/DSVの設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = DirectXCommon::GetInstance()->GetRTVCPUDescriptorHandle(0); // renderTextureResource用
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = DirectXCommon::GetInstance()->GetDSVCPUDescriptorHandle(0);
	commandList->OMSetRenderTargets(1, &offscreenRtvHandle, FALSE, &dsvHandle);
	// クリア
	FLOAT clearColor[4] = {kRenderTargetClearValue.x, kRenderTargetClearValue.y, kRenderTargetClearValue.z, kRenderTargetClearValue.w};
	commandList->ClearRenderTargetView(offscreenRtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// viewportの設定
	D3D12_VIEWPORT viewport = DirectXCommon::GetInstance()->GetViewport();

	// シザー矩形を一時変数に格納
	D3D12_RECT scissorRect = DirectXCommon::GetInstance()->GetScissorRect();

	// ビューポート/シザー設定
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
}

/// <summary>
/// レンダーテクスチャをシェーダリソース用にバリア遷移します。
/// </summary>
void OffScreenRendering::TransitionRenderTextureToShaderResource() {

	renderingBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	renderingBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	renderingBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	commandList->ResourceBarrier(1, &renderingBarrier);
}

/// <summary>
/// レンダーテクスチャをレンダーターゲット用にバリア遷移します。
/// </summary>
void OffScreenRendering::TransitionRenderTextureToRenderTarget() {
    renderingBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    renderingBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    // 深度バッファの直前の状態を正しく指定
    depthBarrier.Transition.StateBefore = depthResourceState_;
    depthBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;

    // バリア発行
    commandList->ResourceBarrier(1, &renderingBarrier);
    commandList->ResourceBarrier(1, &depthBarrier);

    // 状態管理変数を更新
    depthResourceState_ = D3D12_RESOURCE_STATE_DEPTH_WRITE;
}

/// <summary>
/// オフスクリーンテクスチャの描画処理
/// 全画面三角形を描画します。
/// </summary>
void OffScreenRendering::Draw() {

	// 4. SRV用ディスクリプタヒープをセット
	ID3D12DescriptorHeap* descriptorHeaps[] = {DirectXCommon::GetInstance()->GetSrvDescriptorHeap().Get()};
	commandList->SetDescriptorHeaps(1, descriptorHeaps);

	// 5. PSO・ルートシグネチャ設定
	postEffectManager.DrawCurrent(commandList.Get());

	// 6. SRV（オフスクリーンテクスチャ）をルートパラメータにセット
	// ※ルートシグネチャのSRVインデックスに合わせて変更（例: 0番なら0）

	commandList->SetGraphicsRootDescriptorTable(0, DirectXCommon::GetInstance()->GetSRVGPUDescriptorHandle(0));

	// 7. 全画面三角形を描画
	commandList->DrawInstanced(3, 1, 0, 0); // 全画面三角形

		
	// ポストエフェクト描画
	postEffectManager.DrawCurrent(commandList.Get());

}

void OffScreenRendering::DrawImGui() {

	postEffectManager.DrawImGui();

	ImGui::Begin("PostEffect");

	static const char* effectNames[] = {
	    "None",              // 何もしないエフェクト
	    "GrayScale",         // グレースケールエフェクト
	    "Sepia",             // セピアエフェクト
	    "Vignette",          // ビネットエフェクト
	    "Smoothing",         // スムージングエフェクト
	    "GaussianBlur",      // ガウスぼかしエフェクト
	    "RadialBlur",        // ラジアルブラーエフェクト
	    "Outline",           // アウトラインエフェクト
	    "DepthBasedOutline", // 深度ベースのアウトラインエフェクト
	    "Dissolve",          // ディゾルブエフェクト
	    "Random"             // ランダムエフェクト
	                         // 他のエフェクトを追加する場合はここに追加
	};

	int effectIndex = static_cast<int>(postEffectManager.GetCurrentIndex());

	if (ImGui::Combo("Effect", &effectIndex, effectNames, IM_ARRAYSIZE(effectNames))) {
		postEffectManager.SetCurrentEffect(effectIndex);
	}

	ImGui::End();
}

void OffScreenRendering::Finalize() {

	delete instance;
	instance = nullptr;
}

/// <summary>
/// レンダーターゲットリソースの作成
/// 指定されたサイズ・フォーマット・クリアカラーでリソースを生成します。
/// </summary>
/// <param name="device">D3D12デバイス</param>
/// <param name="width">幅</param>
/// <param name="height">高さ</param>
/// <param name="format">フォーマット</param>
/// <param name="clearColor">クリアカラー</param>
/// <returns>作成されたリソース</returns>
Microsoft::WRL::ComPtr<ID3D12Resource>
    OffScreenRendering::CreateRenderTargetResource(Microsoft::WRL::ComPtr<ID3D12Device>& device, int32_t width, int32_t height, DXGI_FORMAT format, const Vector4& clearColor) {
	// リソースの設定
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;  // 2Dテクスチャ
	resourceDesc.Width = static_cast<UINT64>(width);              // テクスチャの幅
	resourceDesc.Height = static_cast<UINT>(height);              // テクスチャの高さ
	resourceDesc.DepthOrArraySize = 1;                            // 奥行きまたは配列サイズ
	resourceDesc.MipLevels = 1;                                   // ミップマップレベル
	resourceDesc.Format = format;                                 // テクスチャフォーマット
	resourceDesc.SampleDesc.Count = 1;                            // サンプリング数（マルチサンプリングなし）
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;           // レイアウト（デフォルト）
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // レンダーターゲットとして使用

	// クリア値の設定
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = format;         // フォーマット
	clearValue.Color[0] = clearColor.x; // クリアカラー (R)
	clearValue.Color[1] = clearColor.y; // クリアカラー (G)
	clearValue.Color[2] = clearColor.z; // クリアカラー (B)
	clearValue.Color[3] = clearColor.w; // クリアカラー (A)

	// ヒーププロパティの設定
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // デフォルトヒープ

	// リソースの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTargetResource;
	HRESULT hr = device->CreateCommittedResource(
	    &heapProperties,      // ヒーププロパティ
	    D3D12_HEAP_FLAG_NONE, // ヒープフラグ
	    &resourceDesc,        // リソース記述子
	    D3D12_RESOURCE_STATE_RENDER_TARGET,
	    // D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,                   // 初期リソースステート
	    &clearValue,                        // クリア値
	    IID_PPV_ARGS(&renderTargetResource) // 作成されたリソース
	);

	// 作成に失敗した場合はエラーを出力
	if (FAILED(hr)) {
		Logger::Log(std::format("Failed to create render target resource. HRESULT = {:#010x}\n", hr));
		assert(SUCCEEDED(hr));
	}

	return renderTargetResource;
}

void OffScreenRendering::TransitionDepthTo(D3D12_RESOURCE_STATES newState) {
    auto depthResource = DirectXCommon::GetInstance()->GetDepthStencliResouece();
    if (!depthResource)
        return;
    if (depthResourceState_ == newState)
        return; // すでに同じ状態なら何もしない

	
    
	depthBarrier.Transition.StateBefore = depthResourceState_;
	depthBarrier.Transition.StateAfter = newState;
	depthBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList->ResourceBarrier(1, &depthBarrier);

    depthResourceState_ = newState; // 必ずここで更新
}