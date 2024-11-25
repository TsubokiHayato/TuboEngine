#pragma once
class ImGuiManager
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(WinApp* winApp);
private:
	//WindowsAppのポインタ
	WinApp* winApp;
};

