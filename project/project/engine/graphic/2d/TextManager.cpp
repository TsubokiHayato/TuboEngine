#include "TextManager.h"
#include <iostream>
#include "externals/imgui/imgui.h"

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

void TextManager::DrawImGui() {
#ifdef USE_IMGUI
    ImGui::Begin("TextManager");

    // フォントの追加UI
    if (ImGui::CollapsingHeader("Add Font")) {
        static char fontNameBuf[64] = "";
        static char fontPathBuf[256] = "C:\\Windows\\Fonts\\msgothic.ttc";
        static float fontSize = 32.0f;

        ImGui::InputText("Font Name", fontNameBuf, sizeof(fontNameBuf));
        ImGui::InputText("Font Path", fontPathBuf, sizeof(fontPathBuf));
        ImGui::DragFloat("Font Size", &fontSize, 1.0f, 8.0f, 128.0f);

        if (ImGui::Button("Load Font")) {
            if (fontNameBuf[0] != '\0' && fontPathBuf[0] != '\0') {
                // char* から std::wstring への変換
                int size_needed = MultiByteToWideChar(CP_UTF8, 0, fontPathBuf, (int)strlen(fontPathBuf), NULL, 0);
                std::wstring wstrPath(size_needed, 0);
                MultiByteToWideChar(CP_UTF8, 0, fontPathBuf, (int)strlen(fontPathBuf), &wstrPath[0], size_needed);

                LoadFont(fontNameBuf, wstrPath, fontSize);
            }
        }
    }

    // 登録済みフォント一覧
    if (ImGui::CollapsingHeader("Loaded Fonts")) {
        for (const auto& pair : fonts_) {
            ImGui::Text("%s", pair.first.c_str());
        }
    }

    // テキストオブジェクトの管理UI
    if (ImGui::CollapsingHeader("Text Objects")) {
        if (ImGui::Button("Create New Text")) {
            if (!fonts_.empty()) {
                CreateText(fonts_.begin()->first, "New Text", { 100.0f, 100.0f });
            }
        }

        ImGui::Separator();

        int index = 0;
        for (auto it = texts_.begin(); it != texts_.end(); ) {
            ImGui::PushID(index);
            
            bool isOpen = ImGui::TreeNode((std::string("Text ") + std::to_string(index)).c_str());
            
            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
                it = texts_.erase(it);
                ImGui::PopID();
                if (isOpen) ImGui::TreePop();
                continue;
            }

            if (isOpen) {
                TextObject* textObj = it->get();

                // テキスト内容の編集
                // std::u32string から UTF-8 への変換は少し手間なので、
                // ここでは簡易的に TextObject 側に GetText() を追加するか、
                // 別の方法で管理する必要があります。
                // 今回は TextObject に GetText() がないと仮定し、
                // 編集用のバッファを別途用意するなどの工夫が必要ですが、
                // ひとまず位置、色、スケールの編集のみ実装します。

                Math::Vector2 pos = textObj->GetPosition();
                if (ImGui::DragFloat2("Position", &pos.x, 1.0f)) {
                    textObj->SetPosition(pos);
                }

                Math::Vector4 color = textObj->GetColor();
                if (ImGui::ColorEdit4("Color", &color.x)) {
                    textObj->SetColor(color);
                }

                float scale = textObj->GetScale();
                if (ImGui::DragFloat("Scale", &scale, 0.01f, 0.1f, 10.0f)) {
                    textObj->SetScale(scale);
                }

                // フォントの変更
                if (!fonts_.empty()) {
                    std::vector<const char*> fontNames;
                    for (const auto& pair : fonts_) {
                        fontNames.push_back(pair.first.c_str());
                    }
                    
                    // 現在のフォントのインデックスを見つける処理は省略（簡易化のため）
                    static int currentFontIdx = 0;
                    if (ImGui::Combo("Font", &currentFontIdx, fontNames.data(), static_cast<int>(fontNames.size()))) {
                        textObj->SetFont(GetFont(fontNames[currentFontIdx]));
                    }
                }

                ImGui::TreePop();
            }
            
            ImGui::PopID();
            ++it;
            ++index;
        }
    }

    ImGui::End();
#endif
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
