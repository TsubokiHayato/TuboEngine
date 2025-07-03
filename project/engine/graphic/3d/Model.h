#pragma once

#include "WinApp.h"

#include "Material.h"
#include "ModelData.h"
#include "Node.h"
#include "Transform.h"
#include "TransformationMatrix.h"
#include "VertexData.h"
#include"DirectXCommon.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

class ModelCommon;
class Model {
public:
	//------------------------------------
	// メンバ関数

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="modelCommon">モデル共通部分</param>
	void Initialize(ModelCommon* modelCommon, const std::string& directoryPath, const std::string& filename);

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
	static ModelData LoadModelFile(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// ノードを読み込む
	/// </summary>
	/// <param name="node">ノード</param>
	/// <returns></returns>
	static Node ReadNode(aiNode* node);

public:
	//-----------------------------------------------------------
	// Setter&Getter

	
	//モデルのカラー
	Vector4 GetModelColor() { return materialData->color; }
	void SetModelColor(const Vector4& color) { materialData->color = color; }

	// 光沢度
	float GetModelShininess() { return materialData->shininess; }
	void SetModelShininess(float shininess) { materialData->shininess = shininess; }

	// rootNodeLocalMatrix
	Matrix4x4 GetRootNodeLocalMatrix() { return modelData.rootNode.localMatrix; }
	void SetRootNodeLocalMatrix(const Matrix4x4& matrix) { modelData.rootNode.localMatrix = matrix; }


private:
	// 共通部分
	ModelCommon* modelCommon_ = nullptr;

	// モデルデータ
	ModelData modelData;

	// 頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	// 頂点バッファ内リソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	// 頂点バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	// マテリアルのバッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	// マテリアルのバッファリソース内のデータを指すポインタ
	Material* materialData = nullptr;

	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;

	std::string textureFileName_;
};
