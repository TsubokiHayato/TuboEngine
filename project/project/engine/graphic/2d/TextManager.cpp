#include "TextManager.h"
#include "externals/imgui/imgui.h"
#include <iostream>
#include <fstream>
#include <externals/nlohmann/json.hpp>

namespace TuboEngine {

using nlohmann::json;

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
#ifdef _WIN32
	windowsFontDir_ = "C:/Windows/Fonts/";
#endif

	// プリセットフォント登録
	LoadFontFromExternal(PresetFontNames::Best10, "BestTen-CRT.otf", 32.0f);
	LoadFontFromExternal(PresetFontNames::SoukouMincho, "SoukouMincho.ttf", 32.0f);
	LoadFontFromExternal(PresetFontNames::UtsukushiFONT, "UtsukushiFONT.otf", 24.0f);
	LoadFontFromExternal(PresetFontNames::YasashisaGothicBold, "YasashisaGothicBold-V2.otf", 48.0f);

	// 必要ならここでデフォルトレイアウト JSON を読み込む
	// LoadTextLayout("Resources/Text/layout.json");
}

void TextManager::Finalize() {
	fonts_.clear();
	texts_.clear();
	textDefs_.clear();
}

void TextManager::UpdateAll() {
	for (auto& text : texts_) {
		text->Update();
	}
}

void TextManager::DrawAll() {
	for (auto& text : texts_) {
		text->Draw();
	}
}

void TextManager::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("TextManager");

	// レイアウトのロード/セーブ
	if (ImGui::CollapsingHeader("Layout")) {
		static char layoutPathBuf[256] = "Resources/Text/layout.json";
		ImGui::InputText("Layout Path", layoutPathBuf, sizeof(layoutPathBuf));
		if (ImGui::Button("Load Layout")) {
			if (!LoadTextLayout(layoutPathBuf)) {
				std::cerr << "Failed to load text layout: " << layoutPathBuf << std::endl;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Save Layout")) {
			if (!SaveTextLayout(layoutPathBuf)) {
				std::cerr << "Failed to save text layout: " << layoutPathBuf << std::endl;
			}
		}
	}

	// フォントの追加UI
	if (ImGui::CollapsingHeader("Add Font")) {
		static char fontNameBuf[64] = "";
		static char fontFileBuf[128] = "msgothic.ttc";
		static float fontSize = 32.0f;
		static int sourceType = 2; // 0: Project, 1: External, 2: Windows

		ImGui::InputText("Font Name", fontNameBuf, sizeof(fontNameBuf));
		ImGui::InputText("Font File", fontFileBuf, sizeof(fontFileBuf));
		ImGui::RadioButton("Project", &sourceType, 0); ImGui::SameLine();
		ImGui::RadioButton("External", &sourceType, 1); ImGui::SameLine();
		ImGui::RadioButton("Windows", &sourceType, 2);
		ImGui::DragFloat("Font Size", &fontSize, 1.0f, 8.0f, 128.0f);

		if (ImGui::Button("Load Font")) {
			if (fontNameBuf[0] != '\0' && fontFileBuf[0] != '\0') {
				Font* font = nullptr;
				std::string fileName(fontFileBuf);

				switch (sourceType) {
				case 0: font = LoadFontFromProject(fontNameBuf, fileName, fontSize); break;
				case 1: font = LoadFontFromExternal(fontNameBuf, fileName, fontSize); break;
				case 2: font = LoadFontFromWindows(fontNameBuf, fileName, fontSize); break;
				default: break;
				}

				if (!font) {
					std::cerr << "Failed to load font via ImGui UI." << std::endl;
				}
			}
		}
	}

	// 登録済みフォント一覧
	if (ImGui::CollapsingHeader("Loaded Fonts")) {
		for (const auto& pair : fonts_) {
			const std::string& name = pair.first;
			bool isPreset =
				name == PresetFontNames::Best10 ||
				name == PresetFontNames::SoukouMincho ||
				name == PresetFontNames::UtsukushiFONT ||
				name == PresetFontNames::YasashisaGothicBold;

			if (isPreset) {
				ImGui::Text("[Preset] %s", name.c_str());
			} else {
				ImGui::Text("%s", name.c_str());
			}
		}
	}

	// テキストオブジェクトの管理UI（定義 + 実体）
	if (ImGui::CollapsingHeader("Text Objects")) {
		if (ImGui::Button("Create New Text")) {
			TextDefinition def{};
			def.name = "Text" + std::to_string(textDefs_.size());
			def.text = "New Text";
			def.fontName = fonts_.empty() ? "" : fonts_.begin()->first;
			def.position = {100.0f, 100.0f};
			def.color = {1.0f, 1.0f, 1.0f, 1.0f};
			def.scale = 1.0f;
			textDefs_.push_back(def);

			// 実体 TextObject も生成
			CreateText(def.fontName, def.text, def.position, def.color, def.scale);
		}

		ImGui::Separator();

		// textDefs_ と texts_ はインデックス対応させる前提
		for (size_t index = 0; index < textDefs_.size() && index < texts_.size();) {
			TextDefinition& def = textDefs_[index];
			TextObject* textObj = texts_[index].get();

			ImGui::PushID(static_cast<int>(index));
			bool isOpen = ImGui::TreeNode(def.name.c_str());
			ImGui::SameLine();
			if (ImGui::Button("Delete")) {
				textDefs_.erase(textDefs_.begin() + index);
				texts_.erase(texts_.begin() + index);
				ImGui::PopID();
				if (isOpen) ImGui::TreePop();
				continue; // index はインクリメントしない
			}

			if (isOpen) {
				// name
				char nameBuf[64];
				std::snprintf(nameBuf, sizeof(nameBuf), "%s", def.name.c_str());
				if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
					def.name = nameBuf;
				}

				// text
				static char textBuf[512];
				std::snprintf(textBuf, sizeof(textBuf), "%s", def.text.c_str());
				if (ImGui::InputTextMultiline("Text", textBuf, sizeof(textBuf))) {
					def.text = textBuf;
					textObj->SetText(def.text);
				}

				// position
				float pos[2] = {def.position.x, def.position.y};
				if (ImGui::DragFloat2("Position", pos, 1.0f)) {
					def.position.x = pos[0];
					def.position.y = pos[1];
					textObj->SetPosition(def.position);
				}

				// color
				float col[4] = {def.color.x, def.color.y, def.color.z, def.color.w};
				if (ImGui::ColorEdit4("Color", col)) {
					def.color = {col[0], col[1], col[2], col[3]};
					textObj->SetColor(def.color);
				}

				// scale
				float scale = def.scale;
				if (ImGui::DragFloat("Scale", &scale, 0.01f, 0.1f, 10.0f)) {
					def.scale = scale;
					textObj->SetScale(def.scale);
				}

				// font
				if (!fonts_.empty()) {
					std::vector<const char*> fontNames;
					int currentFontIdx = 0;
					int i = 0;
					for (const auto& pair : fonts_) {
						fontNames.push_back(pair.first.c_str());
						if (def.fontName == pair.first) {
							currentFontIdx = i;
						}
						i++;
					}

					if (ImGui::Combo("Font", &currentFontIdx, fontNames.data(), static_cast<int>(fontNames.size()))) {
						def.fontName = fontNames[currentFontIdx];
						Font* font = GetFont(def.fontName);
						if (font) {
							textObj->SetFont(font);
						}
					}
				}

				ImGui::TreePop();
			}

			ImGui::PopID();
			++index;
		}
	}

	ImGui::End();
#endif
}

Font* TextManager::LoadFont(const std::string& name, const std::string& filePath, float size) {
	auto it = fonts_.find(name);
	if (it != fonts_.end()) {
		return it->second.get();
	}

	auto font = std::make_unique<Font>();
	std::wstring filePathW(filePath.begin(), filePath.end());
	if (font->Initialize(filePathW, size)) {
		Font* ptr = font.get();
		fonts_[name] = std::move(font);
		return ptr;
	}

	std::cerr << "Failed to load font: " << filePath << std::endl;
	return nullptr;
}

Font* TextManager::LoadFontFromProject(const std::string& name, const std::string& fileName, float size) {
	std::string path = projectFontDir_ + fileName;
	return LoadFont(name, path, size);
}

Font* TextManager::LoadFontFromExternal(const std::string& name, const std::string& fileName, float size) {
	std::string path = externalFontDir_ + fileName;
	return LoadFont(name, path, size);
}

Font* TextManager::LoadFontFromWindows(const std::string& name, const std::string& fileName, float size) {
#ifdef _WIN32
	std::string path = windowsFontDir_ + fileName;
	return LoadFont(name, path, size);
#else
	(void)name; (void)fileName; (void)size;
	std::cerr << "LoadFontFromWindows is only supported on Windows." << std::endl;
	return nullptr;
#endif
}

Font* TextManager::GetFont(const std::string& name) {
	auto it = fonts_.find(name);
	if (it != fonts_.end()) {
		return it->second.get();
	}
	return nullptr;
}

TextObject* TextManager::CreateText(const std::string& fontName, const std::string& text, const Math::Vector2& pos, const Math::Vector4& color, float scale) {
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
	auto it = std::find_if(texts_.begin(), texts_.end(), [text](const std::unique_ptr<TextObject>& ptr) { return ptr.get() == text; });
	if (it != texts_.end()) {
		texts_.erase(it);
	}
}

bool TextManager::LoadTextLayout(const std::string& filePath) {
	std::ifstream ifs(filePath);
	if (!ifs) {
		return false;
	}

	json root;
	try {
		ifs >> root;
	} catch (...) {
		std::cerr << "Failed to parse text layout JSON: " << filePath << std::endl;
		return false;
	}

	if (!root.contains("texts") || !root["texts"].is_array()) {
		return false;
	}

	textDefs_.clear();
	texts_.clear();

	for (auto& jt : root["texts"]) {
		TextDefinition def{};
		def.name     = jt.value("name", "");
		def.text     = jt.value("text", "");
		def.fontName = jt.value("font", PresetFontNames::Best10);
		auto posArr  = jt.value("position", std::vector<float>{0.0f, 0.0f});
		auto colArr  = jt.value("color",    std::vector<float>{1.0f, 1.0f, 1.0f, 1.0f});
		def.scale    = jt.value("scale", 1.0f);

		if (posArr.size() >= 2) {
			def.position = {posArr[0], posArr[1]};
		}
		if (colArr.size() >= 4) {
			def.color = {colArr[0], colArr[1], colArr[2], colArr[3]};
		}

		textDefs_.push_back(def);
	}

	// TextObject を生成
	for (auto& def : textDefs_) {
		CreateText(def.fontName, def.text, def.position, def.color, def.scale);
	}

	return true;
}

bool TextManager::SaveTextLayout(const std::string& filePath) const {
	json root;
	root["texts"] = json::array();

	for (const auto& def : textDefs_) {
		json jt;
		jt["name"]     = def.name;
		jt["text"]     = def.text;
		jt["font"]     = def.fontName;
		jt["position"] = { def.position.x, def.position.y };
		jt["color"]    = { def.color.x, def.color.y, def.color.z, def.color.w };
		jt["scale"]    = def.scale;
		root["texts"].push_back(jt);
	}

	std::ofstream ofs(filePath);
	if (!ofs) {
		return false;
	}
	ofs << root.dump(2);
	return true;
}

} // namespace TuboEngine
