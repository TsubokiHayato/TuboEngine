#include "ImGuiManager.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include <cassert>

void ImGuiManager::Initialize()
{
	
	//ImGuiのコンテキストを作成
	ImGui::CreateContext();
	//ImGuiのスタイルを設定
	ImGui::StyleColorsDark();
	// WinAppが正しく初期化されているか確認
	assert(WinApp::GetInstance()->GetHWND() != nullptr);
	//ImGuiのDirectX12の初期化
	ImGui_ImplWin32_Init(WinApp::GetInstance()->GetHWND());

	//descriptorHeapの設定
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	//descriptorHeapの生成
	HRESULT result = DirectXCommon::GetInstance()->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srvHeap));
	assert(SUCCEEDED(result));

	ImGui_ImplDX12_Init(DirectXCommon::GetInstance()->GetDevice().Get(),
		static_cast<int>(DirectXCommon::GetInstance()->GetBackBufferCount()),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		srvHeap.Get(),
		srvHeap->GetCPUDescriptorHandleForHeapStart(),
		srvHeap->GetGPUDescriptorHandleForHeapStart()
	);
}

void ImGuiManager::Finalize()
{
	//後始末
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();


}

void ImGuiManager::Begin()
{
	//ImGuiの受付開始
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiManager::End()
{//ImGuiの受付終了
	ImGui::Render();
}

void ImGuiManager::Draw()
{
	Microsoft::WRL::ComPtr<	ID3D12GraphicsCommandList> commandList = DirectXCommon::GetInstance()->GetCommandList();

	//ディスクリプタヒープの配列をセットするコマンド
	ID3D12DescriptorHeap* ppHeaps[] = { srvHeap.Get() };	
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	//ImGuiの描画
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
}
