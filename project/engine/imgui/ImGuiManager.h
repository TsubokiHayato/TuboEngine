#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/imgui/imgui_impl_dx12.h"


class ImGuiManager
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(WinApp* winApp);

	/// <summary>
	/// ImGuiの解放
	/// </summary>
	void Finalize();

	/// <summary>
	///ImGui受付開始
	/// </summary>
	void Begin();

	/// <summary>
	/// ImGui受付終了
	///	</summary>
	void End();

	/// <summary>
	/// ImGuiの描画
	/// </summary>
	void Draw();

private:
	//WindowsAppのポインタ
	WinApp* winApp_;
	
	//SRVディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap;
};

