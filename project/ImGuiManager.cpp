#include "ImGuiManager.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "WinApp.h"


void ImGuiManager::Initialize(WinApp* winApp)
{
	//WindowsAppのポインタを受け取る
	this->winApp = winApp;
	//ImGuiのコンテキストを作成
	ImGui::CreateContext();
	//ImGuiのスタイルを設定
	ImGui::StyleColorsDark();
	//ImGuiのDirectX12の初期化
	ImGui_ImplWin32_Init(this->winApp->GetHWND());
}
