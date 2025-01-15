#pragma once


#include"DirectXcommon.h"
#include"WinApp.h"

#include"VertexData.h"
#include"Material.h"
#include"TransformationMatrix.h"
#include"Transform.h"
#include"ModelData.h"
//前方宣言
class Object3dCommon;
class ModelCommon;
class Model;
class Camera;

//平行光源
struct DirectionalLight {
	//色
	Vector4 color;
	//方向
	Vector3 direction;
	//強度
	float intensity;
};


class Object3d
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="object3dCommon"></param>
	void Initialize(Object3dCommon* object3dCommon);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();



	//Setter
	void SetScale(const Vector3& scale) { transform.scale = scale; }
	void SetRotation(const Vector3& rotation) { transform.rotate = rotation; }
	void SetPosition(const Vector3& position) { transform.translate = position; }
	//モデルのセット
	void SetModel(Model* model) {
		assert(model);
		this->model_ = model;
	}

	/// <summary>
	/// 3Dオブジェクトとモデルを紐づける関数
	/// </summary>
	/// <param name="filePath"></param>
	void SetModel(const std::string& filePath);
	void SetCamera(Camera* camera) { this->camera = camera; }

	//Getter
	Vector3 GetScale() const { return transform.scale; }
	Vector3 GetRotation() const { return transform.rotate; }
	Vector3 GetPosition() const { return transform.translate; }



private:


	//共通部分
	Object3dCommon* object3dCommon = nullptr;
	//DirectX共通部分
	DirectXCommon* dxCommon_ = nullptr;
	//ウィンドウズアプリケーション
	WinApp* winApp_ = nullptr;

	//モデル共通部分
	ModelCommon* modelCommon_ = nullptr;
	//モデルデータ
	Model* model_ = nullptr;

	//座標のバッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> transformMatrixResource;
	//座標のバッファリソース内のデータを指すポインタ
	TransformationMatrix* transformMatrixData = nullptr;

	//平行光源のバッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResource;
	//バッファリソース内のデータを指すポインタ
	DirectionalLight* directionalLightData = nullptr;
	//コマンドリスト
	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList;

	//3Dオブジェクトの座標
	Transform transform;
	//カメラ座標
	Transform cameraTransform;

	Camera* camera;

};

