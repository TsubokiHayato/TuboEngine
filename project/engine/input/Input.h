#pragma once
#include <Windows.h>


#include <cassert>

#define DIRECTINPUT_VERSION 0x0800
#include<dinput.h>
#include "WinApp.h"
#include <wrl.h>
class Input
{
private:


	static Input* instance;
	Input() = default;
	~Input() = default;
	Input(Input&) = delete;
	Input& operator=(Input&) = delete;


public:
	//シングルトンインスタンスの取得
	static Input* GetInstance();
	//終了
	void Finalize();

	void Initialize(WinApp* winApp);
	void Update();

	bool PushKey(BYTE keyNumber);
	bool TriggerKey(BYTE keyNumber);



private:


	Microsoft::WRL::ComPtr<IDirectInputDevice8 > keyboard = nullptr;
	Microsoft::WRL::ComPtr<IDirectInput8> directInput = nullptr;

	HRESULT result;
	BYTE key[256] = {};
	BYTE keyPre[256] = {};
	WinApp* winApp_ = nullptr;



};
