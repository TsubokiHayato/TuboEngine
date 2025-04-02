#include "Model.h"
#include "ModelCommon.h"

#include"MT_Matrix.h"
#include"Object3d.h"
#include"TextureManager.h"

#include<cassert>
#include <fstream>
#include<sstream>
#include<filesystem>

#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

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

	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath, aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMeshes());


	/*-----------------------------
	3 : ファイルを読み、ModelDataを構築
	-------------------------------*/
	//ファイルを読み込む
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {

		/*--------------------------------
					メッシュを解析
		--------------------------------*/

		aiMesh* mesh = scene->mMeshes[meshIndex];
		//メッシュの確認
		assert(mesh->HasNormals());
		assert(mesh->HasTextureCoords(0));


		/*--------------------------------
					faceを解析
		---------------------------------*/

		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);//三角形のみ対応

			/*--------------------------------
					vertexを解析
			--------------------------------*/

			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vetexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vetexIndex];
				aiVector3D& normal = mesh->mNormals[vetexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vetexIndex];
				VertexData vertex;
				vertex.position = { position.x,position.y,position.z,1.0f };
				vertex.normal = { normal.x,normal.y,normal.z };
				vertex.texcoord = { texcoord.x,texcoord.y };
				//aiProcess_MakeLeftHandedはz*=-1で、右手->左手に変換するので手動で対処
				vertex.position.x *= -1.0f;
				vertex.normal.x *= -1.0f;
				modelData.vertices.push_back(vertex);
			}

			/*--------------------------------
					materialを解析
			--------------------------------*/

			for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
				aiMaterial* material = scene->mMaterials[materialIndex];
				if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
					aiString texturePath;
					material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
					modelData.material.textureFilePath = directoryPath + "/" + texturePath.C_Str();
				}
			}

		}

	}

	return modelData;

}

