#include "TextManager.h"
#include "externals/imgui/imgui.h"
#include <iostream>
#include <fstream>
#include <externals/nlohmann/json.hpp>
#include <filesystem> // 追加: ディレクトリ作成用

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

	// プリセット BaseFont 登録
	LoadFontFromExternal(PresetFontNames::Best10, "BestTen-CRT.otf", 32.0f);
	LoadFontFromExternal(PresetFontNames::SoukouMincho, "SoukouMincho.ttf", 32.0f);
	LoadFontFromExternal(PresetFontNames::UtsukushiFONT, "UtsukushiFONT.otf", 24.0f);
	LoadFontFromExternal(PresetFontNames::YasashisaGothicBold, "YasashisaGothicBold-V2.otf", 48.0f);

	baseFontNames_.push_back(PresetFontNames::Best10);
	baseFontNames_.push_back(PresetFontNames::SoukouMincho);
	baseFontNames_.push_back(PresetFontNames::UtsukushiFONT);
	baseFontNames_.push_back(PresetFontNames::YasashisaGothicBold);
}

void TextManager::Finalize() {
	fonts_.clear();
	texts_.clear();
	textDefs_.clear();
	baseFontNames_.clear();
	addedFontNames_.clear();
	sizedFontNames_.clear();
}

void TextManager::UpdateAll() {
	for (std::unique_ptr<TextObject>& text : texts_) {
		text->Update();
	}
}

void TextManager::DrawAll() {
	for (std::unique_ptr<TextObject>& text : texts_) {
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

	// フォントの追加UI（AddFont）
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
				std::string fontName(fontNameBuf);
				std::string fileName(fontFileBuf);

				switch (sourceType) {
				case 0: font = LoadFontFromProject(fontName, fileName, fontSize); break;
				case 1: font = LoadFontFromExternal(fontName, fileName, fontSize); break;
				case 2: font = LoadFontFromWindows(fontName, fileName, fontSize); break;
				default: break;
				}

				if (!font) {
					std::cerr << "Failed to load font via ImGui UI." << std::endl;
				} else {
					// AddFont グループに登録（BaseFont とは別）
					addedFontNames_.push_back(fontName);
				}
			}
		}
	}

	// 登録済みフォント一覧（BaseFont / AddFont / SizeFont を区別して表示）
	if (ImGui::CollapsingHeader("Loaded Fonts")) {
		ImGui::Text("BaseFont:");
		for (const std::string& name : baseFontNames_) {
			ImGui::BulletText("%s", name.c_str());
		}
		if (!addedFontNames_.empty()) {
			ImGui::Separator();
			ImGui::Text("AddFont:");
			for (const std::string& name : addedFontNames_) {
				ImGui::BulletText("%s", name.c_str());
			}
		}
		if (!sizedFontNames_.empty()) {
			ImGui::Separator();
			ImGui::Text("SizeFont (generated, not selectable):");
			for (const std::string& name : sizedFontNames_) {
				ImGui::BulletText("%s", name.c_str());
			}
		}
	}

	// テキストオブジェクトの管理UI（定義 + 実体）
	if (ImGui::CollapsingHeader("Text Objects")) {
		if (ImGui::Button("Create New Text")) {
			TextDefinition def{};
			def.name = "Text" + std::to_string(textDefs_.size());
			def.text = "New Text";
			def.fontName = PresetFontNames::Best10;
			def.fontSize = 32.0f;
			def.position = Math::Vector2{100.0f, 100.0f};
			def.color = Math::Vector4{1.0f, 1.0f, 1.0f, 1.0f};
			def.scale = 1.0f;
			textDefs_.push_back(def);

			Font* font = GetOrCreateFontSized(def.fontName, def.fontSize);
			if (font) {
				TextObject* obj = CreateText(def.fontName + "_" + std::to_string(static_cast<int>(def.fontSize)), def.text, def.position, def.color, def.scale);
				if (obj) {
					obj->SetFont(font);
				}
			}
		}

		ImGui::Separator();

		for (size_t index = 0; index < textDefs_.size() && index < texts_.size();) {
			TextDefinition& def = textDefs_[index];
			TextObject* textObj = texts_[index].get();

			ImGui::PushID(static_cast<int>(index));
			// ツリーラベル: 表示名 + 固定ID + index（ImGui の ID は ## 以降）
			char treeLabel[128];
			std::snprintf(treeLabel, sizeof(treeLabel), "TextObject [%s]##TextDef%zu", def.name.c_str(), index);
			bool isOpen = ImGui::TreeNode(treeLabel);
			ImGui::SameLine();
			if (ImGui::Button("Delete")) {
				textDefs_.erase(textDefs_.begin() + static_cast<std::ptrdiff_t>(index));
				texts_.erase(texts_.begin() + static_cast<std::ptrdiff_t>(index));
				ImGui::PopID();
				if (isOpen) {
					ImGui::TreePop();
				}
				continue;
			}

			if (isOpen) {
				// name
				char nameBuf[64];
				std::snprintf(nameBuf, sizeof(nameBuf), "%s", def.name.c_str());
				if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
					def.name = nameBuf;
				}

				// text: use a local fixed buffer each frame, seeded from def.text
				{
					char textBuf[1024];
					std::size_t len = def.text.size();
					if (len >= sizeof(textBuf)) { len = sizeof(textBuf) - 1; }
					std::memcpy(textBuf, def.text.data(), len);
					textBuf[len] = '\0';
					if (ImGui::InputTextMultiline("Text", textBuf, sizeof(textBuf))) {
						def.text = textBuf;
						textObj->SetText(def.text);
					}
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
					def.color = Math::Vector4{col[0], col[1], col[2], col[3]};
					textObj->SetColor(def.color);
				}

				// scale（見た目のスケール調整）
				float scale = def.scale;
				if (ImGui::DragFloat("Scale", &scale, 0.01f, 0.1f, 10.0f)) {
					def.scale = scale;
					textObj->SetScale(def.scale);
				}

				// BaseFont + AddFont から選択（SizeFont は対象外）
				std::vector<const char*> selectableFontNames;
				std::vector<std::string> selectableNamesStorage;
				// BaseFont
				for (const std::string& n : baseFontNames_) {
					selectableNamesStorage.push_back(n);
				}
				// AddFont
				for (const std::string& n : addedFontNames_) {
					selectableNamesStorage.push_back(n);
				}
				for (std::string& s : selectableNamesStorage) {
					selectableFontNames.push_back(s.c_str());
				}

				int baseIndex = 0;
				for (int i = 0; i < static_cast<int>(selectableNamesStorage.size()); ++i) {
					if (def.fontName == selectableNamesStorage[i]) {
						baseIndex = i;
						break;
					}
				}
				if (!selectableFontNames.empty()) {
					if (ImGui::Combo("Base/Add Font", &baseIndex,
						selectableFontNames.data(),
						static_cast<int>(selectableFontNames.size()))) {
						def.fontName = selectableNamesStorage[baseIndex];
						Font* sizedFont = GetOrCreateFontSized(def.fontName, def.fontSize);
						if (sizedFont) {
							textObj->SetFont(sizedFont);
						}
					}
				}

				// フォントサイズ（実サイズ）
				float fontSize = def.fontSize;
				if (ImGui::DragFloat("Font Size", &fontSize, 1.0f, 8.0f, 128.0f)) {
					def.fontSize = fontSize;
					Font* sizedFont = GetOrCreateFontSized(def.fontName, def.fontSize);
					if (sizedFont) {
						textObj->SetFont(sizedFont);
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
	std::unordered_map<std::string, std::unique_ptr<Font>>::iterator it = fonts_.find(name);
	if (it != fonts_.end()) {
		return it->second.get();
	}

	std::unique_ptr<Font> font = std::make_unique<Font>();
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
	std::unordered_map<std::string, std::unique_ptr<Font>>::iterator it = fonts_.find(name);
	if (it != fonts_.end()) {
		return it->second.get();
	}
	return nullptr;
}

Font* TextManager::GetOrCreateFontSized(const std::string& baseName, float fontSize) {
	std::string sizedName = baseName + "_" + std::to_string(static_cast<int>(fontSize));

	Font* existing = GetFont(sizedName);
	if (existing) {
		return existing;
	}

	std::string fileName;
	if (baseName == PresetFontNames::Best10) {
		fileName = "BestTen-CRT.otf";
	} else if (baseName == PresetFontNames::SoukouMincho) {
		fileName = "SoukouMincho.ttf";
	} else if (baseName == PresetFontNames::UtsukushiFONT) {
		fileName = "UtsukushiFONT.otf";
	} else if (baseName == PresetFontNames::YasashisaGothicBold) {
		fileName = "YasashisaGothicBold-V2.otf";
	} else {
		// AddFont からの baseName は、そのまま external ファイル名とみなす
		fileName = baseName;
	}

	Font* sized = LoadFontFromExternal(sizedName, fileName, fontSize);
	if (sized) {
		sizedFontNames_.push_back(sizedName);
	}
	return sized;
}

TextObject* TextManager::CreateText(const std::string& fontName, const std::string& text, const Math::Vector2& pos, const Math::Vector4& color, float scale) {
	Font* font = GetFont(fontName);
	if (!font) {
		std::cerr << "Font not found: " << fontName << std::endl;
		return nullptr;
	}

	std::unique_ptr<TextObject> textObj = std::make_unique<TextObject>();
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
	std::vector<std::unique_ptr<TextObject>>::iterator it = std::find_if(
		texts_.begin(), texts_.end(),
		[text](const std::unique_ptr<TextObject>& ptr) { return ptr.get() == text; });
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

	for (json& jt : root["texts"]) {
		TextDefinition def{};
		def.name     = jt.value("name", "");
		def.text     = jt.value("text", "");
		def.fontName = jt.value("font", PresetFontNames::Best10);
		def.fontSize = jt.value("fontSize", 32.0f);
		std::vector<float> posArr  = jt.value("position", std::vector<float>{0.0f, 0.0f});
		std::vector<float> colArr  = jt.value("color",    std::vector<float>{1.0f, 1.0f, 1.0f, 1.0f});
		def.scale    = jt.value("scale", 1.0f);

		if (posArr.size() >= 2) {
			def.position = Math::Vector2{posArr[0], posArr[1]};
		}
		if (colArr.size() >= 4) {
			def.color = Math::Vector4{colArr[0], colArr[1], colArr[2], colArr[3]};
		}

		textDefs_.push_back(def);
	}

	// TextObject を生成
	for (TextDefinition& def : textDefs_) {
		Font* font = GetOrCreateFontSized(def.fontName, def.fontSize);
		if (font) {
			TextObject* obj = CreateText(def.fontName + "_" + std::to_string(static_cast<int>(def.fontSize)), def.text, def.position, def.color, def.scale);
			if (obj) {
			 obj->SetFont(font);
			}
		}
	}

	return true;
}

bool TextManager::SaveTextLayout(const std::string& filePath) const {
	json root;
	root["texts"] = json::array();

	for (const TextDefinition& def : textDefs_) {
		json jt;
		jt["name"]     = def.name;
		jt["text"]     = def.text;
		jt["font"]     = def.fontName;
		jt["fontSize"] = def.fontSize;
		jt["position"] = { def.position.x, def.position.y };
		jt["color"]    = { def.color.x, def.color.y, def.color.z, def.color.w };
		jt["scale"]    = def.scale;
		root["texts"].push_back(jt);
	}

	// 親ディレクトリが無ければ作成
	try {
		std::filesystem::path path(filePath);
		if (path.has_parent_path()) {
			std::filesystem::create_directories(path.parent_path());
		}
	} catch (const std::exception& e) {
		std::cerr << "Failed to create directories for layout: " << e.what() << std::endl;
		return false;
	}

	std::ofstream ofs(filePath);
	if (!ofs) {
		std::cerr << "Failed to open layout file for writing: " << filePath << std::endl;
		return false;
	}
	ofs << root.dump(2);
	return true;
}

} // namespace TuboEngine
