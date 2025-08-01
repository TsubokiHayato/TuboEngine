#include "BlenderSceneLoader.h"
#include <fstream>
#include <memory>
#include <stdexcept>
#include "externals/nlohmann/json.hpp"
#include"ImGuiManager.h"
#include <windows.h>

namespace {

constexpr std::string_view kDefaultBaseDirectory = "resources/levels/stage/";
constexpr std::string_view kExtension = ".json";
constexpr std::string_view kObjExtension = ".obj";

// 子オブジェクトも含めて再帰的にパース
void ParseObject(const nlohmann::json& object, LevelData& levelData) {
    if (!object.contains("type")) return;
    const std::string& type = object["type"];

    if (type == "MESH") {
        if (object.value("disabled_flag", false)) return;

        LevelData::ObjectData objData;
        objData.fileName = object.value("file_name", object.value("name", "")) + std::string(kObjExtension);

        if (object.contains("transform")) {
            const auto& transform = object["transform"];
            if (!transform.contains("translation")) {
                OutputDebugStringA("transform: translation not found\n");
            }
            if (!transform.contains("rotation")) {
                OutputDebugStringA("transform: rotation not found\n");
            }
            if (!transform.contains("scaling")) {
                OutputDebugStringA("transform: scaling not found\n");
            }
            if (transform.contains("translation") && !transform["translation"].is_array()) {
                OutputDebugStringA("transform: translation is not array\n");
            }
            if (transform.contains("rotation") && !transform["rotation"].is_array()) {
                OutputDebugStringA("transform: rotation is not array\n");
            }
            if (transform.contains("scaling") && !transform["scaling"].is_array()) {
                OutputDebugStringA("transform: scaling is not array\n");
            }
            if (transform.contains("translation") && transform["translation"].is_array() && transform["translation"].size() < 3) {
                OutputDebugStringA("transform: translation size < 3\n");
            }
            if (transform.contains("rotation") && transform["rotation"].is_array() && transform["rotation"].size() < 3) {
                OutputDebugStringA("transform: rotation size < 3\n");
            }
            if (transform.contains("scaling") && transform["scaling"].is_array() && transform["scaling"].size() < 3) {
                OutputDebugStringA("transform: scaling size < 3\n");
            }
            if (transform.contains("translation") && transform["translation"].is_array() && transform["translation"].size() >= 3 &&
                transform.contains("rotation") && transform["rotation"].is_array() && transform["rotation"].size() >= 3 &&
                transform.contains("scale") && transform["scale"].is_array() && transform["scale"].size() >= 3) {
                objData.transform.translate.x = static_cast<float>(transform["translation"][0].get<double>());
                objData.transform.translate.y = static_cast<float>(transform["translation"][2].get<double>());
                objData.transform.translate.z = static_cast<float>(transform["translation"][1].get<double>());
                objData.transform.rotate.x = static_cast<float>(transform["rotation"][0].get<double>());
                objData.transform.rotate.y = static_cast<float>(transform["rotation"][2].get<double>());
                objData.transform.rotate.z = static_cast<float>(transform["rotation"][1].get<double>());
                objData.transform.scale.x = static_cast<float>(transform["scale"][0].get<double>());
                objData.transform.scale.y = static_cast<float>(transform["scale"][2].get<double>());
                objData.transform.scale.z = static_cast<float>(transform["scale"][1].get<double>());
				if (transform.contains("translation") && transform["translation"].is_array() && transform["translation"].size() >= 3) {
					float tx = static_cast<float>(transform["translation"][0].get<double>());
					float ty = static_cast<float>(transform["translation"][2].get<double>());
					float tz = static_cast<float>(transform["translation"][1].get<double>());
					printf("Parse: %s pos=(%f,%f,%f)\n", objData.fileName.c_str(), tx, ty, tz);
					objData.transform.translate.x = tx;
					objData.transform.translate.y = ty;
					objData.transform.translate.z = tz;
				}
            }
        }

        levelData.objects.push_back(std::move(objData));
    }
    else if (type == "PlayerSpawn") {
        if (object.value("disabled_flag", false)) return;

        LevelData::PlayerSpawnData spawnData;
        spawnData.fileName = object.value("file_name", object.value("name", "")) + std::string(kObjExtension);

        const auto& transform = object["transform"];
        spawnData.transform.translate.x = transform["translation"][0];
        spawnData.transform.translate.y = transform["translation"][2];
        spawnData.transform.translate.z = transform["translation"][1];
        spawnData.transform.rotate.x = transform["rotation"][0];
        spawnData.transform.rotate.y = transform["rotation"][2];
        spawnData.transform.rotate.z = transform["rotation"][1];
        spawnData.transform.scale.x = transform["scaling"][0];
        spawnData.transform.scale.y = transform["scaling"][2];
        spawnData.transform.scale.z = transform["scaling"][1];

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
                if (transform.contains("translation") && transform["translation"].is_array() && transform["translation"].size() >= 3 &&
                    transform.contains("rotation") && transform["rotation"].is_array() && transform["rotation"].size() >= 3 &&
                    transform.contains("scale") && transform["scale"].is_array() && transform["scale"].size() >= 3) {
                    levelData->cameraTransform.translate.x = static_cast<float>(transform["translation"][0].get<double>());
                    levelData->cameraTransform.translate.y = static_cast<float>(transform["translation"][2].get<double>());
                    levelData->cameraTransform.translate.z = static_cast<float>(transform["translation"][1].get<double>());
                    levelData->cameraTransform.rotate.x = static_cast<float>(transform["rotation"][0].get<double>());
                    levelData->cameraTransform.rotate.y = static_cast<float>(transform["rotation"][2].get<double>());
                    levelData->cameraTransform.rotate.z = static_cast<float>(transform["rotation"][1].get<double>());
                    levelData->cameraTransform.scale.x = static_cast<float>(transform["scale"][0].get<double>());
                    levelData->cameraTransform.scale.y = static_cast<float>(transform["scale"][2].get<double>());
                    levelData->cameraTransform.scale.z = static_cast<float>(transform["scale"][1].get<double>());
                }
            }
        }
    }
}

void BlenderSceneLoader::CreateObject() {
    objects.clear();
    for (const auto& objectData : levelData->objects) {
        printf("CreateObject: %s pos=(%f,%f,%f)\n", objectData.fileName.c_str(),
            objectData.transform.translate.x, objectData.transform.translate.y, objectData.transform.translate.z);
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