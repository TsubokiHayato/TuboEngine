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
	void Initialize(Object3dCommon* object3dCommon,WinApp* winApp, DirectXCommon* dxCommon);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// マテリアルテンプレートファイルを読み込む
	/// </summary>
	/// <param name="directoryPath">ディレクトリパス</param>
	/// <param name="filePath">ファイルパス</param>
	/// <returns>マテリアルデータ</returns>
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filePath);


	/// <summary>
	/// OBJファイルを読み込む
	/// </summary>
	/// <param name="directoryPath">ディレクトリパス</param>
	/// <param name="filename">ファイル名</param>
	/// <returns>モデルデータ</returns>
	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

	void SetModel(Model* model) {
		this->model_ = model;
	}

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

};

