#pragma once
#include"DirectXcommon.h"
#include"WinApp.h"

#include"VertexData.h"
#include"Material.h"
#include"TransformationMatrix.h"
#include"Transform.h"
#include"ModelData.h"



class ModelCommon;
class Model
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="modelCommon">モデル共通部分</param>
	void Initialize(ModelCommon* modelCommon,const std::string& directoryPath,const std::string& filename);
	
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


	void SetModelColor(const Vector4& color) {
		materialData->color = color;
	}

	Vector4 GetModelColor() {
		return materialData->color;
	}

	//光沢度
	float GetModelShininess() {
		return materialData->shininess;
	}
	void SetModelShininess(float shininess) {
		materialData->shininess = shininess;
	}

	
private:
	//共通部分
	ModelCommon* modelCommon_ = nullptr;

	//モデルデータ
	ModelData modelData;

	//頂点リソースを作る
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource;
	//頂点バッファ内リソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	//頂点バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	//マテリアルのバッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> materialResource;
	//マテリアルのバッファリソース内のデータを指すポインタ
	Material* materialData = nullptr;

	//コマンドリスト
	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList;


};

