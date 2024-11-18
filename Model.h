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
	void Initialize(ModelCommon* modelCommon);
	
	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();
private:
	DirectXCommon* dxCommon_ = nullptr;
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

};

