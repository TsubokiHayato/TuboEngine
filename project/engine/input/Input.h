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

	//初期化
	void Initialize();
	//更新
	void Update();

	/// <summary>
	/// キーが押されているか
	/// </summary>
	/// <param name="keyNumber">キー番号</param>
	/// <returns>押されているか</returns>
	bool PushKey(BYTE keyNumber);

	/// <summary>
	/// キーが押された瞬間か
	/// </summary>
	/// <param name="keyNumber">キー番号</param>
	/// <returns>押された瞬間か</returns>
	bool TriggerKey(BYTE keyNumber);



private:

	//キーボード
	Microsoft::WRL::ComPtr<IDirectInputDevice8 > keyboard = nullptr;
	//ダイレクトインプット
	Microsoft::WRL::ComPtr<IDirectInput8> directInput = nullptr;

	//キーボードの状態
	HRESULT result;
	//現在のキーボードの状態
	BYTE key[256] = {};
	//前のキーの状態
	BYTE keyPre[256] = {};
	

};
