#include "Input.h"

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
Input* Input::instance = nullptr;


Input* Input::GetInstance()
{
	// インスタンスがない場合は作成する
    if (instance == nullptr) {
        instance = new Input;
    }
    return instance;
}

void Input::Finalize()
{
	// インスタンスがある場合は削除する
    delete instance;
    instance = nullptr;
}


void Input::Initialize()
{

    HRESULT result;
	
    // DirectInputのインスタンス生成
    result = DirectInput8Create(WinApp::GetInstance()->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
    assert(SUCCEEDED(result));

    // キーボードデバイスの作成

    result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
    assert(SUCCEEDED(result));

    // 入力データ形式のセット
    result = keyboard->SetDataFormat(&c_dfDIKeyboard);
    assert(SUCCEEDED(result));

    // 協調レベルのセット
    result = keyboard->SetCooperativeLevel(WinApp::GetInstance()->GetHWND(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
    assert(SUCCEEDED(result));
}
void Input::Update()
{
	// キーボードの状態を取得
    if (key != nullptr && keyPre != nullptr) {
		// 前のキーの状態を保存
        memcpy(keyPre, key, sizeof(key));
    }
	// キーボードの状態を取得
    keyboard->Acquire();
    HRESULT result = keyboard->GetDeviceState(sizeof(key), key);

}
bool Input::PushKey(BYTE keyNumber)
{
	// キーが押されているか
    if (key[keyNumber]) {
        return true;
    }
    return false;
}
bool Input::TriggerKey(BYTE keyNumber)
{
	// キーが押された瞬間か
    return (key[keyNumber] != 0) && (keyPre[keyNumber] == 0);
}