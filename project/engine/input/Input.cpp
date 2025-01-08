#include "Input.h"

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
Input* Input::instance = nullptr;

Input* Input::GetInstance()
{
    if (instance == nullptr) {
        instance = new Input;
    }
    return instance;
}

void Input::Finalize()
{
    delete instance;
    instance = nullptr;
}


void Input::Initialize(WinApp* winApp)
{
    HRESULT result;

    this->winApp_ = winApp;

    // DirectInputのインスタンス生成
    result = DirectInput8Create(winApp->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
    assert(SUCCEEDED(result));

    // キーボードデバイスの作成

    result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
    assert(SUCCEEDED(result));

    // 入力データ形式のセット
    result = keyboard->SetDataFormat(&c_dfDIKeyboard);
    assert(SUCCEEDED(result));

    // 協調レベルのセット
    result = keyboard->SetCooperativeLevel(winApp->GetHWND(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
    assert(SUCCEEDED(result));
}
void Input::Update()
{
    if (key != nullptr && keyPre != nullptr) {
        memcpy(keyPre, key, sizeof(key));
    }
    keyboard->Acquire();
    HRESULT result = keyboard->GetDeviceState(sizeof(key), key);

}
bool Input::PushKey(BYTE keyNumber)
{
    if (key[keyNumber]) {
        return true;
    }
    return false;
}
bool Input::TriggerKey(BYTE keyNumber)
{
    return (key[keyNumber] != 0) && (keyPre[keyNumber] == 0);
}