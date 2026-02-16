#pragma once

#include <Windows.h>
#include <XInput.h>
#include <array>
#include <cstdint>
#include <dinput.h>
#include <vector>
#include <wrl.h>
#include "engine/math/Vector2.h"

/**
 * @brief 入力管理クラス。
 *
 * @details
 * `Input` はアプリケーション全体の入力（キーボード/マウス/ゲームパッド）を統合して管理します。
 * 毎フレーム `Update()` を呼ぶことで、
 * - 現在フレームの入力状態
 * - 1フレーム前の入力状態
 * を更新し、押下/トリガー判定やマウス移動量、パッド接続状態などを提供します。
 *
 * @note
 * DirectInput と XInput を併用し、接続されているパッドの種類に応じて状態を保持します。
 */
class Input {
public:
	using Vector2 = TuboEngine::Math::Vector2;

	struct MouseMove {
		LONG lX;
		LONG lY;
		LONG lZ;
	};

	enum class PadType {
		DirectInput,
		XInput,
	};

	union State {
		XINPUT_STATE xInput_;
		DIJOYSTATE2 directInput_;
		State() { ZeroMemory(this, sizeof(State)); }
	};

	struct Joystick {
		Microsoft::WRL::ComPtr<IDirectInputDevice8> device_;
		int32_t deadZoneL_ = 0;
		int32_t deadZoneR_ = 0;
		PadType type_;
		State state_;
		State statePre_;
		bool isConnected_ = false;
		int32_t xinputIndex_ = -1;
	};

public:
	static Input* GetInstance();

	void Initialize(HWND hwnd);
	void Update();
	void Finalize();
	void ShowInputDebugWindow();

	// キーボード
	/**
	 * @brief キーが押されているか（押下中）を取得します。
	 * @param keyNumber DirectInputのDIKコード（例: `DIK_ESCAPE`）。
	 * @return 押下中ならtrue。
	 */
	bool PushKey(BYTE keyNumber) const;
	/**
	 * @brief キーが押された瞬間（トリガー）かを取得します。
	 * @param keyNumber DirectInputのDIKコード（例: `DIK_SPACE`）。
	 * @return 今フレームで押されたならtrue（前フレームは未押下）。
	 */
	bool TriggerKey(BYTE keyNumber) const;
	/**
	 * @brief キー配列（256キー）の生データを取得します。
	 * @return 256要素のキー状態配列（DirectInput形式）。
	 */
	const std::array<BYTE, 256>& GetAllKey() const { return key_; }

	// 入力のトリガーを消す（ポーズ解除などで、直前の押下判定を残さないため）
	/**
	 * @brief トリガー判定用の前フレーム状態を現在状態で上書きします。
	 * @details ポーズ解除直後など「直前の押下」を残したくない場面で使用します。
	 */
	void FlushTriggers();

	// マウス
	/** @brief マウスの生状態（ボタン/移動/ホイール）を取得します。 */
	const DIMOUSESTATE2& GetAllMouse() const { return mouse_; }
	/**
	 * @brief マウスボタンが押下中かを取得します。
	 * @param button ボタンID（0:左 / 1:右 / 2:中 ...）。
	 * @return 押下中ならtrue。
	 */
	bool IsPressMouse(int32_t button) const;
	/**
	 * @brief マウスボタンが押された瞬間（トリガー）かを取得します。
	 * @param button ボタンID（0:左 / 1:右 / 2:中 ...）。
	 * @return 今フレームで押されたならtrue（前フレームは未押下）。
	 */
	bool IsTriggerMouse(int32_t button) const;
	/**
	 * @brief 1フレーム分のマウス移動量を取得します。
	 * @return 
	 * - `lX`, `lY`: 移動量（相対）
	 * - `lZ`: ホイール移動量
	 */
	MouseMove GetMouseMove() const;
	/**
	 * @brief マウスホイールの移動量を取得します（1フレーム分）。
	 * @return ホイール移動量（DirectInputの値）。
	 */
	int32_t GetWheel() const;
	/**
	 * @brief 現在のマウス座標（クライアント座標）を取得します。
	 * @return マウス座標。
	 */
	const Vector2& GetMousePosition() const { return mousePosition_; }

	// ジョイスティック
	/**
	 * @brief DirectInputパッドの状態を取得します。
	 * @param stickNo パッド番号（0..）。
	 * @param[out] out 取得した状態（DirectInput）。
	 * @return 取得できた（接続されている）場合true。
	 */
	bool GetJoystickState(int32_t stickNo, DIJOYSTATE2& out) const;
	/**
	 * @brief DirectInputパッドの前フレーム状態を取得します。
	 * @param stickNo パッド番号（0..）。
	 * @param[out] out 前フレーム状態（DirectInput）。
	 * @return 取得できた（接続されている）場合true。
	 */
	bool GetJoystickStatePrevious(int32_t stickNo, DIJOYSTATE2& out) const;
	/**
	 * @brief XInputパッドの状態を取得します。
	 * @param stickNo パッド番号（0..）。
	 * @param[out] out 取得した状態（XInput）。
	 * @return 取得できた（接続されている）場合true。
	 */
	bool GetJoystickState(int32_t stickNo, XINPUT_STATE& out) const;
	/**
	 * @brief XInputパッドの前フレーム状態を取得します。
	 * @param stickNo パッド番号（0..）。
	 * @param[out] out 前フレーム状態（XInput）。
	 * @return 取得できた（接続されている）場合true。
	 */
	bool GetJoystickStatePrevious(int32_t stickNo, XINPUT_STATE& out) const;
	/**
	 * @brief スティックのデッドゾーンを設定します。
	 * @param stickNo パッド番号（0..）。
	 * @param deadZoneL 左スティックのデッドゾーン。
	 * @param deadZoneR 右スティックのデッドゾーン。
	 */
	void SetJoystickDeadZone(int32_t stickNo, int32_t deadZoneL, int32_t deadZoneR);
	/**
	 * @brief 検出されたジョイスティックの数を取得します。
	 * @return ジョイスティック数。
	 */
	size_t GetNumberOfJoysticks() const;
	/**
	 * @brief パッドが接続されているかを取得します。
	 * @param stickNo パッド番号（0..）。
	 * @return 接続されていればtrue。
	 */
	bool IsPadConnected(int32_t stickNo) const;

private:
	static Input* instance;
	Input() = default;
	~Input() = default;
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;

	void SetupJoysticks();
	static BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext) noexcept;

private:
	// DirectInput
	Microsoft::WRL::ComPtr<IDirectInput8> dInput_;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> devKeyboard_;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> devMouse_;
	std::vector<Joystick> devJoysticks_;

	// キーボード
	std::array<BYTE, 256> key_ = {};
	std::array<BYTE, 256> keyPre_ = {};

	// マウス
	DIMOUSESTATE2 mouse_ = {};
	DIMOUSESTATE2 mousePre_ = {};
	Vector2 mousePosition_ = {};

	// ウィンドウハンドル
	HWND hwnd_ = nullptr;
};
