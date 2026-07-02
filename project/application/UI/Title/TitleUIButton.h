#pragma once
#include <functional>
#include <string>

/// <summary>
/// タイトル画面のメニューボタン1個分。ラベルとクリック時のコールバックを保持する。
/// </summary>
class TitleUIButton {
public:
	TitleUIButton(const std::string& label, std::function<void()> onClick) : label_(label), onClick_(onClick) {}

	/// <summary>
	/// 描画処理。
	/// </summary>
	void Draw(int index, bool selected) const;
	/// <summary>
	/// クリック時のコールバックを実行する。
	/// </summary>
	void Click() const {
		if (onClick_)
			onClick_();
	}
	/// <summary>
	/// Label を取得する。
	/// </summary>
	const std::string& GetLabel() const { return label_; }

private:
	std::string label_;
	std::function<void()> onClick_;
};