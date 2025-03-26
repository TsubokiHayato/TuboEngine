#pragma once
#include"DirectXcommon.h"
#include"WinApp.h"

#include"VertexData.h"
#include"Material.h"
#include"TransformationMatrix.h"
#include"Transform.h"
#include"ModelData.h"
#include"BlendMode.h"
#include"CameraForGPU.h"
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

//PointLight
struct PointLight
{
	//色
	Vector4 color;
	//位置
	Vector3 position;
	//輝度
	float intensity;
};

struct SpotLight
{
	//色
	Vector4 color;
	//位置
	Vector3 position;
	//輝度
	float intensity;
	//方向
	Vector3 direction;
	//ライトの届く最大距離
	float distance;
	//減衰率
	float decay;
	//スポットライトの余弦
	float cosAngle;
	//
	float padding[2];
};

struct LightType {

	//0 : 平行光源
	//1 : Phong反射モデル
	//2 : Blinn-Phong反射モデル
	//3 : PointLight
	//4 : SpotLight
	int type;

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

public:

	//Setter
	void SetScale(const Vector3& scale) { transform.scale = scale; }
	void SetRotation(const Vector3& rotation) { transform.rotate = rotation; }
	void SetPosition(const Vector3& position) { transform.translate = position; }

	///-------------------------------------------------------------------------------------------------
	/// Light
	//平行光源
	void SetLightColor(const Vector4& color) { directionalLightData->color = color; }
	void SetLightDirection(const Vector3& direction) { directionalLightData->direction = direction; }
	void SetLightIntensity(float intensity) { directionalLightData->intensity = intensity; }
	void SetLightShininess(float shininess);
	//ポイントライト
	void SetPointLightPosition(const Vector3& position) { pointLightData->position = position; }
	void SetPointLightColor(const Vector4& color) { pointLightData->color = color; }
	void SetPointLightIntensity(float intensity) { pointLightData->intensity = intensity; }
	//スポットライト
	void SetSpotLightColor(const Vector4& color) { spotLightData->color = color; }
	void SetSpotLightPosition(const Vector3& position) { spotLightData->position = position; }
	void SetSpotLightDirection(const Vector3& direction) { spotLightData->direction = direction; }
	void SetSpotLightIntensity(float intensity) { spotLightData->intensity = intensity; }
	void SetSpotLightDistance(float distance) { spotLightData->distance = distance; }
	void SetSpotLightDecay(float decay) { spotLightData->decay = decay; }
	void SetSpotLightCosAngle(float cosAngle) { spotLightData->cosAngle = cosAngle; }


	void SetLightType(int type) {
		if (type < 0 || type > 4) {
			type = 0;
		}
		lightTypeData->type = type;
	}

	void SetModel(Model* model) {
		assert(model);
		this->model_ = model;
	}
	void SetModel(const std::string& filePath);
	void SetCamera(Camera* camera) { this->camera = camera; }

	void SetModelColor(const Vector4& color);

	//Getter
	Vector3 GetScale() const { return transform.scale; }
	Vector3 GetRotation() const { return transform.rotate; }
	Vector3 GetPosition() const { return transform.translate; }
	//モデルの色
	Vector4 GetModelColor();

	///-------------------------------------------------------------------------------------------------
	/// Light
	//平行光源
	Vector4 GetLightColor() { return directionalLightData->color; }
	Vector3 GetLightDirection() { return directionalLightData->direction; }
	float GetLightIntensity() { return directionalLightData->intensity; }
	int GetLightType() { return lightTypeData->type; }
	float GetLightShininess();
	//ポイントライト
	Vector3 GetPointLightPosition() { return pointLightData->position; }
	Vector4 GetPointLightColor() { return pointLightData->color; }
	float GetPointLightIntensity() { return pointLightData->intensity; }
	//スポットライト
	void GetSpotLightColor(Vector4& color) { color = spotLightData->color; }
	void GetSpotLightPosition(Vector3& position) { position = spotLightData->position; }
	void GetSpotLightDirection(Vector3& direction) { direction = spotLightData->direction; }
	float GetSpotLightIntensity() { return spotLightData->intensity; }
	float GetSpotLightDistance() { return spotLightData->distance; }
	float GetSpotLightDecay() { return spotLightData->decay; }
	float GetSpotLightCosAngle() { return spotLightData->cosAngle; }











private:

	//-------------------------------------------------------------------
	//		メンバ変数

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
	//カメラ
	Camera* camera;



	//座標のバッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> transformMatrixResource;
	//座標のバッファリソース内のデータを指すポインタ
	TransformationMatrix* transformMatrixData = nullptr;

	//平行光源のバッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResource;
	//バッファリソース内のデータを指すポインタ
	DirectionalLight* directionalLightData = nullptr;

	//ポイントライトのバッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> pointLightResource;
	//バッファリソース内のデータを指すポインタ
	PointLight* pointLightData = nullptr;

	//スポットライトのバッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> spotLightResource;
	//バッファリソース内のデータを指すポインタ
	SpotLight* spotLightData = nullptr;


	//コマンドリスト
	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList;

	//カメラ座標のバッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> cameraForGPUResource
		= nullptr;
	//カメラ座標のバッファリソース内のデータを指すポインタ
	CameraForGPU* cameraForGPUData = nullptr;

	//ライトの種類
	Microsoft::WRL::ComPtr <ID3D12Resource> lightTypeResource= nullptr;
	//ライトの種類のバッファリソース内のデータを指すポインタ
	LightType* lightTypeData = nullptr;

	//3Dオブジェクトの座標
	Transform transform;
	//カメラ座標
	Transform cameraTransform;


	

};

