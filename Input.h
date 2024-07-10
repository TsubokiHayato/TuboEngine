#pragma once
#include <Windows.h>

#include<wrl.h>

#include <cassert>

#define DIRECTINPUT_VERSION 0x0800
#include<dinput.h>
#include "WinApp.h"

class Input
{


public:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	void Initialize(WinApp* winApp);
	void Update();

	bool PushKey(BYTE keyNumber);
	bool TriggerKey(BYTE keyNumber);



private:


	ComPtr<IDirectInputDevice8> keyboard = nullptr;
	ComPtr<IDirectInput8> directInput = nullptr;

	HRESULT result;
	BYTE key[256] = {};
	BYTE keyPre[256] = {};
	WinApp* winApp_ = nullptr;


};

