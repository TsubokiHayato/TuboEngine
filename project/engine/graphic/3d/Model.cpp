#include "Model.h"
#include "ModelCommon.h"

#include"MT_Matrix.h"
#include"Object3d.h"
#include"TextureManager.h"

#include<cassert>
#include <fstream>
#include<sstream>
#include<filesystem>

void Model::Initialize(ModelCommon* modelCommon, const std::string& directoryPath, const std::string& filename)
{

	//引数で受け取ってメンバ変数に記録する
	this->modelCommon_ = modelCommon;
	
	commandList =modelCommon_->GetDxCommon()->GetCommandList();
	


#pragma region ModelData
	//モデル読み込み
	modelData = LoadObjFile(directoryPath, filename);
	//頂点リソースを作る
	vertexResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	//頂点バッファビューを作成する
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	//頂点リソースにデータを書き込む
	vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
#pragma endregion ModelData
	

#pragma region Material_Resource
	//マテリアル用のリソースを作る。今回はColor1つ分のサイズを用意する
	materialResource =
		modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む
	materialData = nullptr;
	//書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//今回は赤を書き込んでみる
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = true;
	materialData->uvTransform = MakeIdentity4x4();
	materialData->shininess = 70.0f;

#pragma endregion Material_Resource
	

	// テクスチャファイル名を抽出
	textureFileName_ = std::filesystem::path(modelData.material.textureFilePath).filename().string();

	// テクスチャを読み込む
	TextureManager::GetInstance()->LoadTexture(textureFileName_);
	modelData.material.textureIndex = TextureManager::GetInstance()->GetSrvIndex(textureFileName_);
}

void Model::Draw()
{
	
	//頂点バッファをセット
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	//マテリアルバッファをセット
	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	commandList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureFileName_));
	//描画
	commandList->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);

}



MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filePath)
{

	/*------------------------
	1 : 中で必要になる変数の宣言
	------------------------*/

	MaterialData materialData;
	std::string line;


	/*------------------------
	2 : ファイルを開く
	------------------------*/

	std::ifstream file(directoryPath + "/" + filePath);
	assert(file.is_open());

	/*---------------------------------------------
	3 : 実際にファイルを読み、MaterialDataを構築していく
	---------------------------------------------*/

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;

			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}


	/*------------------------
	4 : MaterialDataを返す
	------------------------*/

	return materialData;

}



ModelData Model::LoadObjFile(const std::string& directoryPath, const std::string& filename)
{


	/*-------------
	1 : OBJファイル
	--------------*/
	ModelData modelData;//構築する
	std::vector<Vector4> positions;//位置
	std::vector<Vector3> normals;//法線
	std::vector<Vector2> texcoords;//テクスチャ座標
	std::string line;//ファイルから読んだ1行を格納するもの


	/*----------------------
	2 : OBJファイルを読み込む
	----------------------*/
	std::ifstream file(directoryPath + "/" + filename); //ファイルを読み込む
	assert(file.is_open());//開けなかったら止める



	/*-----------------------------
	3 : ファイルを読み、ModelDataを構築
	-------------------------------*/

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;//先頭の識別子を読む


		//identifierに応じた処理


		/*------------------
			頂点情報を読む
		------------------*/
		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}

		/*---------------
			三角形を作る
		----------------*/
		else if (identifier == "f") {


			VertexData triangle[3];

			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;

				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');
					elementIndices[element] = std::stoi(index);
				}

				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];

				position.x *= -1.0f;
				normal.x *= -1.0f;
				texcoord.y = 1.0f - texcoord.y;


				/*VertexData vertex = { position,texcoord,normal };
				modelData.vertices.push_back(vertex);*/

				triangle[faceVertex] = { position,texcoord,normal };
			}

			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib") {
			//materialTemplateLibraryの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}


	return modelData;

}

