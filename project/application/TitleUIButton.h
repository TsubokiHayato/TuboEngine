#pragma once
#include <functional>
#include <string>

class TitleUIButton {
public:
	TitleUIButton(const std::string& label, std::function<void()> onClick) : label_(label), onClick_(onClick) {}

	void Draw(int index, bool selected) const;
	void Click() const {
		if (onClick_)
			onClick_();
	}
	const std::string& GetLabel() const { return label_; }

private:
	std::string label_;
	std::function<void()> onClick_;
};