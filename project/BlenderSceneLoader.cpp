#include "BlenderSceneLoader.h"
#include "ImGuiManager.h"
#include "externals/nlohmann/json.hpp"
#include <fstream>
#include <memory>
#include <stdexcept>
#include <windows.h>

namespace {

constexpr std::string_view kDefaultBaseDirectory = "resources/levels/stage/";
constexpr std::string_view kExtension = ".json";
constexpr std::string_view kObjExtension = ".gltf";


inline Vector3 ConvertPosition(const nlohmann::json& arr) {
	// Blender: [x, y, z] → Engine: x, z, y
	return {static_cast<float>(arr[0].get<double>()), static_cast<float>(arr[2].get<double>()), static_cast<float>(arr[1].get<double>())};
}

inline Vector3 ConvertRotation(const nlohmann::json& arr) { 
	return {-static_cast<float>(arr[0].get<double>()),- static_cast<float>(arr[2].get<double>()), -static_cast<float>(arr[1].get<double>())}; 
}
inline Vector3 ConvertScale(const nlohmann::json& arr) {
	// Blender: [x, y, z] → Engine: x, z, y
	return {static_cast<float>(arr[0].get<double>()), static_cast<float>(arr[2].get<double>()), static_cast<float>(arr[1].get<double>())};
}

// 子オブジェクトも含めて再帰的にパース
void ParseObject(const nlohmann::json& object, LevelData& levelData) {
	// 無効化フラグ対応
	if (object.contains("Disabled")) {
		bool disabled = object["Disabled"].get<bool>();
		if (disabled) {
			return;
		}
	}

	if (!object.contains("type"))
		return;
	const std::string& type = object["type"];

	if (type == "MESH") {
		if (object.value("disabled_flag", false))
			return;

		LevelData::ObjectData objData;
		objData.fileName = object.value("file_name", object.value("name", "")) + std::string(kObjExtension);

		if (object.contains("transform")) {
			const auto& transform = object["transform"];
			// 変換チェック
			if (transform.contains("translation") && transform["translation"].is_array() && transform["translation"].size() >= 3 && transform.contains("rotation") &&
			    transform["rotation"].is_array() && transform["rotation"].size() >= 3 && (transform.contains("scale") && transform["scale"].is_array() && transform["scale"].size() >= 3)) {

				objData.transform.translate = ConvertPosition(transform["translation"]);
				objData.transform.rotate = ConvertRotation(transform["rotation"]);
				objData.transform.scale = ConvertScale(transform["scale"]);

				// デバッグ出力
				printf(
				    "Parse: %s pos=(%f,%f,%f) rot=(%f,%f,%f) scale=(%f,%f,%f)\n", objData.fileName.c_str(), objData.transform.translate.x, objData.transform.translate.y, objData.transform.translate.z,
				    objData.transform.rotate.x, objData.transform.rotate.y, objData.transform.rotate.z, objData.transform.scale.x, objData.transform.scale.y, objData.transform.scale.z);
			}
		}

		levelData.objects.push_back(std::move(objData));
	} else if (type == "PlayerSpawn") {
		if (object.value("disabled_flag", false))
			return;

		LevelData::PlayerSpawnData spawnData;
		spawnData.fileName = object.value("file_name", object.value("name", "")) + std::string(kObjExtension);

		const auto& transform = object["transform"];
		if (transform.contains("translation") && transform["translation"].is_array() && transform["translation"].size() >= 3 && transform.contains("rotation") && transform["rotation"].is_array() &&
		    transform["rotation"].size() >= 3 && (transform.contains("scaling") && transform["scaling"].is_array() && transform["scaling"].size() >= 3)) {

			spawnData.transform.translate = ConvertPosition(transform["translation"]);
			spawnData.transform.rotate = ConvertRotation(transform["rotation"]);
			spawnData.transform.scale = ConvertScale(transform["scaling"]);
		}

		levelData.playerSpawn.push_back(std::move(spawnData));
	}

	// 子オブジェクトの再帰処理
	if (object.contains("Children")) {
		for (const auto& child : object["Children"]) {
			ParseObject(child, levelData);
		}
	}
}

} // namespace

void BlenderSceneLoader::Load(const std::string& fileName) {
	const std::string fullPath = std::string(kDefaultBaseDirectory) + fileName + std::string(kExtension);

	std::ifstream file(fullPath);
	if (!file) {
		throw std::runtime_error("Failed to open level file: " + fullPath);
	}

	nlohmann::json deserialized;
	file >> deserialized;

	if (!deserialized.is_object() || !deserialized.contains("name") || deserialized["name"] != "scene") {
		throw std::runtime_error("Invalid level file format: " + fullPath);
	}

	levelData = std::make_unique<LevelData>();

	if (deserialized.contains("objects")) {
		for (const auto& object : deserialized["objects"]) {
			ParseObject(object, *levelData);

			// カメラオブジェクトを検出してTransformを保存
			if (object.contains("type") && object["type"] == "CAMERA" && object.contains("transform")) {
				const auto& transform = object["transform"];
				if (transform.contains("translation") && transform["translation"].is_array() && transform["translation"].size() >= 3 && transform.contains("rotation") &&
				    transform["rotation"].is_array() && transform["rotation"].size() >= 3 && (transform.contains("scale") && transform["scale"].is_array() && transform["scale"].size() >= 3)) {

					levelData->cameraTransform.translate = ConvertPosition(transform["translation"]);
					levelData->cameraTransform.rotate = ConvertRotation(transform["rotation"]);
					levelData->cameraTransform.scale = ConvertScale(transform["scale"]);
				}
			}
		}
	}
}

void BlenderSceneLoader::CreateObject() {
	objects.clear();
	for (const auto& objectData : levelData->objects) {
		printf("CreateObject: %s pos=(%f,%f,%f)\n", objectData.fileName.c_str(), objectData.transform.translate.x, objectData.transform.translate.y, objectData.transform.translate.z);
		auto obj = std::make_unique<Object3d>();
		obj->Initialize(objectData.fileName);
		obj->SetPosition(objectData.transform.translate);
		obj->SetRotation(objectData.transform.rotate);
		obj->SetScale(objectData.transform.scale);
		objects.push_back(std::move(obj));
	}
}

void BlenderSceneLoader::Update() {
	for (auto& object : objects) {
		if (object) {
			object->Update();
		}
	}
}

void BlenderSceneLoader::Draw() {
	for (auto& object : objects) {
		object->Draw();
	}
}

void BlenderSceneLoader::DrawImgui() {
#ifdef _DEBUG
	int i = 0;
	for (auto& object : objects) {
		if (object) {
			std::string windowName = "Object3d[" + std::to_string(i) + "]";
			ImGui::Begin(windowName.c_str());
			Vector3 pos = object->GetPosition();
			if (ImGui::DragFloat3("Position", &pos.x, 0.1f))
				object->SetPosition(pos);
			Vector3 rot = object->GetRotation();
			if (ImGui::DragFloat3("Rotation", &rot.x, 0.1f))
				object->SetRotation(rot);
			Vector3 scale = object->GetScale();
			if (ImGui::DragFloat3("Scale", &scale.x, 0.1f))
				object->SetScale(scale);
			ImGui::End();
		}
		++i;
	}
#endif
}

const std::vector<LevelData::PlayerSpawnData>& BlenderSceneLoader::getPlayerSpawns() const {
	assert(levelData);
	return levelData->playerSpawn;
}

bool BlenderSceneLoader::HasPlayerSpawn() const {
	assert(levelData);
	return !levelData->playerSpawn.empty();
}

void BlenderSceneLoader::SetCamera(Camera* camera) {
	for (auto& obj : objects) {
		if (obj) {
			obj->SetCamera(camera);
		}
	}
}