#include "TextManager.h"
#include <iostream>

namespace TuboEngine {

TextManager* TextManager::instance_ = nullptr;

TextManager* TextManager::GetInstance() {
    if (instance_ == nullptr) {
        instance_ = new TextManager();
    }
    return instance_;
}

void TextManager::DestroyInstance() {
    if (instance_) {
        instance_->Finalize();
        delete instance_;
        instance_ = nullptr;
    }
}

void TextManager::Initialize() {
    // SpriteCommonの描画設定を流用するため、ここでは特別な初期化は不要
    // 必要に応じてテキスト専用の初期化処理を追加
}

void TextManager::Finalize() {
    fonts_.clear();
    texts_.clear();

    // シングルトンインスタンスの削除はここで行わない
    // delete instance_;
    // instance_ = nullptr;
}

void TextManager::UpdateAll() {
    for (auto& text : texts_) {
        text->Update();
    }
}

void TextManager::DrawAll() {
    // SpriteCommonの描画設定を流用
    // 描画前にSpriteCommon::DrawSettingsCommon(1) (NormalBlend) などを呼び出す想定
    // ここでは各TextObjectのDrawを呼び出すのみ

    for (auto& text : texts_) {
        text->Draw();
    }
}

Font* TextManager::LoadFont(const std::string& name, const std::wstring& filePath, float size) {
    auto it = fonts_.find(name);
    if (it != fonts_.end()) {
        return it->second.get();
    }

    auto font = std::make_unique<Font>();
    if (font->Initialize(filePath, size)) {
        Font* ptr = font.get();
        fonts_[name] = std::move(font);
        return ptr;
    }

    return nullptr;
}

Font* TextManager::GetFont(const std::string& name) {
    auto it = fonts_.find(name);
    if (it != fonts_.end()) {
        return it->second.get();
    }
    return nullptr;
}

TextObject* TextManager::CreateText(
    const std::string& fontName,
    const std::string& text,
    const Math::Vector2& pos,
    const Math::Vector4& color,
    float scale
) {
    Font* font = GetFont(fontName);
    if (!font) {
        std::cerr << "Font not found: " << fontName << std::endl;
        return nullptr;
    }

    auto textObj = std::make_unique<TextObject>();
    textObj->Initialize();
    textObj->SetFont(font);
    textObj->SetText(text);
    textObj->SetPosition(pos);
    textObj->SetColor(color);
    textObj->SetScale(scale);

    TextObject* ptr = textObj.get();
    texts_.push_back(std::move(textObj));
    return ptr;
}

void TextManager::RemoveText(TextObject* text) {
    auto it = std::find_if(texts_.begin(), texts_.end(),
        [text](const std::unique_ptr<TextObject>& ptr) { return ptr.get() == text; });
    if (it != texts_.end()) {
        texts_.erase(it);
    }
}

} // namespace TuboEngine
