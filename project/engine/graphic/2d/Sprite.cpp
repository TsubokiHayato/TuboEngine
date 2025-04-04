#include "Sprite.h"
#include"SpriteCommon.h"
#include"MT_Matrix.h"
#include"TextureManager.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/imgui/imgui_impl_dx12.h"

void Sprite::Initialize(SpriteCommon* spriteCommon, std::string textureFilePath)
{
	this->spriteCommon = spriteCommon;
	this->dxCommon_ = this->spriteCommon->GetDxCommon();
	this->winApp_ =this->spriteCommon->GetWinApp();

	textureFilePath_ = textureFilePath;

	TextureManager::GetInstance()->LoadTexture(textureFilePath);

#pragma region SpriteResource

	vertexResource = dxCommon_->CreateBufferResource(sizeof(VertexData) * 6);

	//頂点バッファビューを作成する

	//リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6;
	//1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);


	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	//三角形を2つ使って、四角形の作る
	/*---------------
		B-------D		1枚目 : ABCの三角形
		|		|		2枚目 : BDCの三角形
		|		|
		A-------C
	----------------*/
	//A
	vertexData[0].position = { 0.0f,360.0f,0.0f,1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };
	vertexData[0].normal = { 0.0f,0.0f,1.0f };

	//B

	vertexData[1].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexData[1].texcoord = { 0.0f,0.0f };
	vertexData[1].normal = { 0.0f,0.0f,1.0f };

	//C

	vertexData[2].position = { 640.0f,360.0f,0.0f,1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };
	vertexData[2].normal = { 0.0f,0.0f,1.0f };


	//D
	vertexData[3].position = { 640.0f,0.0f,0.0f,1.0f };
	vertexData[3].texcoord = { 1.0f,0.0f };
	vertexData[3].normal = { 0.0f,0.0f,1.0f };


	transformationMatrixResource = dxCommon_->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込む
	transformationMatrixData = nullptr;

	//書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	//単位行列を書き込んでおく
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();


#pragma endregion


#pragma region indexResourceSprite

	//WVP用のリソースを作る
	indexResource = dxCommon_->CreateBufferResource(sizeof(uint32_t) * 6);




	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();

	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;


	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	indexData[0] = 0;    indexData[1] = 1;   indexData[2] = 2;
	indexData[3] = 1;    indexData[4] = 3;   indexData[5] = 2;



#pragma endregion



#pragma region Material_Resource_Sprite
	//マテリアル用のリソースを作る。今回はColor1つ分のサイズを用意する
	materialResource =
		dxCommon_->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む
	//書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//今回は白を書き込んでみる
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();


#pragma endregion


	textureIndex = TextureManager::GetInstance()->GetSrvIndex(textureFilePath);


}

void Sprite::Update()
{

	//textureの位置
	float left = 0.0f - anchorPoint.x;
	float right = 1.0f - anchorPoint.x;
	float top = 0.0f - anchorPoint.y;
	float bottom = 1.0f - anchorPoint.y;


	// 左右反転
	if (isFlipX_) {
		left = 1.0f - anchorPoint.x;
		right = 0.0f - anchorPoint.x;
	}
	else {
		left = 0.0f - anchorPoint.x;
		right = 1.0f - anchorPoint.x;
	}
	//上下反転
	if (isFlipY_) {
		top = 1.0f - anchorPoint.x;
		bottom = 0.0f - anchorPoint.x;
	}
	else {
		top = 0.0f - anchorPoint.x;
		bottom = 1.0f - anchorPoint.x;
	}


	//テクスチャのメタデータを取得
	const DirectX::TexMetadata& metadata =
		TextureManager::GetInstance()->GetMetaData(textureFilePath_);

	//テクスチャの初期サイズ時の座標
	float tex_left = textureLeftTop_.x / metadata.width;
	float tex_right = (textureLeftTop_.x + textureSize_.x) / metadata.width;
	float tex_top = textureLeftTop_.y / metadata.height;
	float tex_bottom = (textureLeftTop_.y + textureSize_.y) / metadata.height;
	
	//テクスチャの初期サイズを呼び出す関数
	if (isAdjustTextureSize) {
		AdjustTextureSize();
	}
	else {
		textureSize_ = { 100.0f,100.0f };
		size = textureSize_;
	}

	/*---------------------------------------
	テクスチャの位置、画像位置, 法線ベクトル, 大きさ
	---------------------------------------*/

	transform.translate = { position.x,position.y,0.0f };
	transform.rotate = { 0.0f,0.0f,rotation };

	vertexData[0].position = { left,bottom,0.0f,1.0f };
	vertexData[0].texcoord = { tex_left,tex_bottom };
	vertexData[0].normal = { 0.0f,0.0f,-1.0f };

	vertexData[1].position = { left,top,0.0f,1.0f };
	vertexData[1].texcoord = { tex_left,tex_top };
	vertexData[1].normal = { 0.0f,0.0f,-1.0f };

	vertexData[2].position = { right,bottom,0.0f,1.0f };
	vertexData[2].texcoord = { tex_right,tex_bottom };
	vertexData[2].normal = { 0.0f,0.0f,-1.0f };

	vertexData[3].position = { right,top,0.0f,1.0f };
	vertexData[3].texcoord = { tex_right,tex_top };
	vertexData[3].normal = { 0.0f,0.0f,-1.0f };

	transform.scale = { size.x,size.y,1.0f };


	/*---------
	行列更新処理
	---------*/
	Matrix4x4 uvTransformMatrix = MakeAffineMatrix(uvTransFormMatrix.scale, uvTransFormMatrix.rotate, uvTransFormMatrix.translate);

	materialData->uvTransform = uvTransformMatrix;

	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, float(winApp_->kClientWidth), float(winApp_->kClientHeight), 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData->WVP = worldViewProjectionMatrix;
	transformationMatrixData->World = worldMatrix;



	commandList = dxCommon_->GetCommandList();




}


void Sprite::Draw(){

	

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);

	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	//TransformationMatrixCBufferの設定
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

	commandList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureFilePath_));
	//描画
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

}

void Sprite::AdjustTextureSize()
{

	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureFilePath_);

	textureSize_.x = static_cast<float>(metadata.width);
	textureSize_.y = static_cast<float>(metadata.height);
	
	size = textureSize_;
}
