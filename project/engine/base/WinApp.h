#pragma once

#include<cstdint>
#include<windows.h>




class WinApp
{

public:

	void Finalize();

	void Initialize();
	void Update();

	HWND GetHWND() const { return hwnd; }
	HINSTANCE GetHInstance() const { return wc.hInstance; }
	MSG GetMSG()const { return msg; }

	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

	bool ProcessMessage();

	//static LRESULT CALLBACK CallWindowProc(HWND hwnd, UINT msg,
	//	WPARAM wparam, LPARAM lparam);
private:

	//クライアント領域のサイズ

	//ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc;

	//ウィンドウの生成
	HWND hwnd = nullptr;

	WNDCLASS wc{};
	MSG msg{};

};

