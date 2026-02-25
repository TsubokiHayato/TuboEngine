#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "Font.h"
#include "TextObject.h"
#include "Vector2.h"
#include "Vector4.h"

namespace TuboEngine {

class TextManager {
public:
    static TextManager* GetInstance();
    static void DestroyInstance();

    void Initialize();
    void Finalize();
    void UpdateAll();
    void DrawAll();
    void DrawImGui();

    // フォント管理
    Font* LoadFont(const std::string& name, const std::wstring& filePath, float size);
    Font* GetFont(const std::string& name);

    // テキスト作成/削除
	TextObject* CreateText(
        const std::string& fontName,
        const std::string& text,
        const Math::Vector2& pos,
        const Math::Vector4& color = {1.0f, 1.0f, 1.0f, 1.0f},
        float scale = 1.0f
    );
    void RemoveText(TextObject* text);

private:
    TextManager() = default;
    ~TextManager() = default;
    TextManager(const TextManager&) = delete;
    TextManager& operator=(const TextManager&) = delete;

    static TextManager* instance_;

    std::unordered_map<std::string, std::unique_ptr<Font>> fonts_;
	std::vector<std::unique_ptr<TextObject>> texts_;
};

} // namespace TuboEngine
