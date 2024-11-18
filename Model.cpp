#include "Model.h"
#include "ModelCommon.h"
#include"MT_Matrix.h"
#include"Object3d.h"
#include"TextureManager.h"
void Model::Initialize(ModelCommon* modelCommon)
{
	//引数で受け取ってメンバ変数に記録する
	this->modelCommon_ = modelCommon;
	DirectXCommon* dxCommon = modelCommon_->GetDxCommon();

#pragma region ModelData
	//モデル読み込み
	modelData = Object3d::LoadObjFile("Resources", "plane.obj");
	//頂点リソースを作る
	vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
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
		dxCommon->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む
	materialData = nullptr;
	//書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//今回は赤を書き込んでみる
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = true;
	materialData->uvTransform = MakeIdentity4x4();

#pragma endregion Material_Resource


	//.Objの参照にしているテクスチャファイルを読み込む
	TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);
	//読み込んだテクスチャの番号を取得する
	modelData.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData.material.textureFilePath);


}

void Model::Draw()
{
	auto commandList = modelCommon_->GetDxCommon()->GetCommandList();
	//頂点バッファをセット
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	commandList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(modelData.material.textureIndex));

	//マテリアルバッファをセット
	commandList->SetGraphicsRootConstantBufferView(2, materialResource->GetGPUVirtualAddress());
	//描画
	commandList->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);

}
