#include "SkyBox.h"
#include "TextureManager.h"
#include "MT_Matrix.h"

void SkyBox::Initialize(SkyBoxCommon* skyBoxCommon, const std::string& textureFilePath) {
	// 引数で受け取ってメンバ変数に記録する
	this->skyBoxCommon = skyBoxCommon;
	
	textureFilePath_ = textureFilePath;//このテクスチャはdds形式であることを想定している

	// テクスチャロード
	TextureManager::GetInstance()->LoadTexture(textureFilePath_);



	// 頂点バッファ
	vertexResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * 24);
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 24;
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// 頂点データをまとめて初期化
	const VertexData vertices[24] = {
		// 右面
		{{ 1.0f,  1.0f,  1.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }},
		{{ 1.0f,  1.0f, -1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }},
		{{ 1.0f, -1.0f,  1.0f, 1.0f }, { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }},
		{{ 1.0f, -1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }},
		// 左面
		{{ -1.0f,  1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }},
		{{ -1.0f,  1.0f,  1.0f, 1.0f }, { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }},
		{{ -1.0f, -1.0f, -1.0f, 1.0f }, { 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }},
		{{ -1.0f, -1.0f,  1.0f, 1.0f }, { 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }},
		// 前面
		{{ -1.0f,  1.0f,  1.0f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }},
		{{  1.0f,  1.0f,  1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }},
		{{ -1.0f, -1.0f,  1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }},
		{{  1.0f, -1.0f,  1.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }},
		// 背面
		{{ -1.0f,  1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }},
		{{  1.0f,  1.0f, -1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }},
		{{ -1.0f, -1.0f, -1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }},
		{{  1.0f, -1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }},
		// 上面
		{{ -1.0f,  1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }},
		{{  1.0f,  1.0f, -1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }},
		{{ -1.0f,  1.0f,  1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }},
		{{  1.0f,  1.0f,  1.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }},
		// 下面
		{{ -1.0f, -1.0f,  1.0f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }},
		{{  1.0f, -1.0f,  1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }},
		{{ -1.0f, -1.0f, -1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }},
		{{  1.0f, -1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }},
	};

	// 頂点データをコピー
	memcpy(vertexData, vertices, sizeof(vertices));


#pragma region indexResourceSprite

	// インデックスバッファ（36個分）を作成
	indexResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t) * 36);
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 36;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// インデックスデータをマップ
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	// 各面2三角形×6面分のインデックスを初期化
	uint32_t indices[] = {
		// 右面
		0, 1, 2, 2, 1, 3,
		// 左面
		4, 5, 6, 6, 5, 7,
		// 前面
		8, 9,10,10, 9,11,
		// 背面
	   12,13,14,14,13,15,
	   // 上面
	  16,17,18,18,17,19,
	  // 下面
	 20,21,22,22,21,23
	};
	memcpy(indexData, indices, sizeof(indices));

#pragma endregion



	// transformationMatrixResource_ の生成
	transformationMatrixResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();


#pragma region Material_Resource_Sprite
	//マテリアル用のリソースを作る。今回はColor1つ分のサイズを用意する
	materialResource =
		DirectXCommon::GetInstance()->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む
	//書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//今回は白を書き込んでみる
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();


#pragma endregion


	//skyBoxの大きさが小さいので、スケールを大きくする
	transform.scale = { 1000.0f, 1000.0f, 1000.0f };
}

void SkyBox::Update() {
	/*---------
	行列更新処理
	---------*/
	// 行列を更新する
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 cameraMatrix = MakeAffineMatrix(camera_->GetScale(), camera_->GetRotation(), camera_->GetTranslate());
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = MakePerspectiveMatrix(0.45f, float(skyBoxCommon->GetWinApp()->kClientWidth) / float(skyBoxCommon->GetWinApp()->kClientHeight), 0.1f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

	if (camera_) {
		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	} else {
		worldViewProjectionMatrix = worldMatrix;
	}

	transformationMatrixData->WVP = worldMatrix * Multiply(viewMatrix, projectionMatrix);
	transformationMatrixData->World = worldMatrix;

	commandList = DirectXCommon::GetInstance()->GetCommandList();
}

void SkyBox::Draw() {

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);

	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	//TransformationMatrixCBufferの設定
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());

	commandList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureFilePath_));
	//描画
	commandList->DrawIndexedInstanced(24, 1, 0, 0, 0);


}
