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

    // よく使うフォント名
    struct PresetFontNames {
        static inline const std::string Best10             = "Best10";
        static inline const std::string SoukouMincho       = "SoukouMincho";
        static inline const std::string UtsukushiFONT      = "UtsukushiFONT";     // UI用小サイズ
        static inline const std::string YasashisaGothicBold = "YasashisaGothicBold-V2"; // UI用大サイズ
    };

    // プリセットフォントへのショートカット
    Font* GetBest10Font()           { return GetFont(PresetFontNames::Best10); }
    Font* GetSoukouMinchoFont()     { return GetFont(PresetFontNames::SoukouMincho); }
    Font* GetUtsukushiFONT()        { return GetFont(PresetFontNames::UtsukushiFONT); }
    Font* GetYasashisaGothicBoldV2(){ return GetFont(PresetFontNames::YasashisaGothicBold); }

    // テキスト定義（保存用）
    struct TextDefinition {
        std::string name;        // 識別名
        std::string text;        // 表示文字列 (UTF-8)
        std::string fontName;    // 使用フォント名
        Math::Vector2 position{0.0f, 0.0f};
        Math::Vector4 color{1.0f, 1.0f, 1.0f, 1.0f};
        float scale = 1.0f;
    };

    // フォント管理
    Font* LoadFont(const std::string& name, const std::string& filePath, float size = 32.0f);
    Font* GetFont(const std::string& name);

    // プロジェクト標準フォント (Resources/Font/)
    Font* LoadFontFromProject(const std::string& name, const std::string& fileName, float size = 32.0f);
    // 外部フォント (Resources/Font/external/)
    Font* LoadFontFromExternal(const std::string& name, const std::string& fileName, float size = 32.0f);
    // Windows フォントディレクトリから読み込み (Windows のみ)
    Font* LoadFontFromWindows(const std::string& name, const std::string& fileName, float size = 32.0f);

    // テキスト作成/削除
    TextObject* CreateText(
        const std::string& fontName,
        const std::string& text,
        const Math::Vector2& pos,
        const Math::Vector4& color = {1.0f, 1.0f, 1.0f, 1.0f},
        float scale = 1.0f
    );
    void RemoveText(TextObject* text);

    // JSON レイアウトのロード/セーブ
    bool LoadTextLayout(const std::string& filePath);
    bool SaveTextLayout(const std::string& filePath) const;

private:
    TextManager() = default;
    ~TextManager() = default;
    TextManager(const TextManager&) = delete;
    TextManager& operator=(const TextManager&) = delete;

    static TextManager* instance_;

    std::unordered_map<std::string, std::unique_ptr<Font>> fonts_;
    std::vector<std::unique_ptr<TextObject>> texts_;

    // 保存用のテキスト定義と TextObject の対応
    std::vector<TextDefinition> textDefs_;

    // フォント検索用ディレクトリ
    std::string projectFontDir_ = "Resources/Font/";
    std::string externalFontDir_ = "Resources/Font/external/";
    std::string windowsFontDir_{}; // Initialize で設定 (Windows のみ)
};

} // namespace TuboEngine
