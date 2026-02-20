#include "Sprite.h"
#include"SpriteCommon.h"
#include"Matrix.h"
#include"TextureManager.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/imgui/imgui_impl_dx12.h"

void Sprite::Initialize(std::string textureFilePath)
{
	
	
	textureFilePath_ = textureFilePath;

	TextureManager::GetInstance()->LoadTexture(textureFilePath);

#pragma region SpriteResource

	vertexResource_ = TuboEngine::DirectXCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * 6);

	//頂点バッファビューを作成する

	//リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 6;
	//1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);


	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	//三角形を2つ使って、四角形の作る
	/*---------------
		B-------D		1枚目 : ABCの三角形
		|		|		2枚目 : BDCの三角形
		|		|
		A-------C
	----------------*/
	//A
	vertexData_[0].position = { 0.0f,360.0f,0.0f,1.0f };
	vertexData_[0].texcoord = { 0.0f,1.0f };
	vertexData_[0].normal = { 0.0f,0.0f,1.0f };

	//B

	vertexData_[1].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexData_[1].texcoord = { 0.0f,0.0f };
	vertexData_[1].normal = { 0.0f,0.0f,1.0f };

	//C

	vertexData_[2].position = { 640.0f,360.0f,0.0f,1.0f };
	vertexData_[2].texcoord = { 1.0f,1.0f };
	vertexData_[2].normal = { 0.0f,0.0f,1.0f };


	//D
	vertexData_[3].position = { 640.0f,0.0f,0.0f,1.0f };
	vertexData_[3].texcoord = { 1.0f,0.0f };
	vertexData_[3].normal = { 0.0f,0.0f,1.0f };


	transformationMatrixResource_ = TuboEngine::DirectXCommon::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込む
	transformationMatrixData_ = nullptr;

	//書き込むためのアドレスを取得
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	//単位行列を書き込んでおく
	transformationMatrixData_->WVP = MakeIdentity4x4();
	transformationMatrixData_->World = MakeIdentity4x4();


#pragma endregion


#pragma region indexResourceSprite

	//WVP用のリソースを作る
	indexResource_ = TuboEngine::DirectXCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t) * 6);
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();

	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;


	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));

	indexData_[0] = 0;    indexData_[1] = 1;   indexData_[2] = 2;
	indexData_[3] = 1;    indexData_[4] = 3;   indexData_[5] = 2;



#pragma endregion



#pragma region Material_Resource_Sprite
	//マテリアル用のリソースを作る。今回はColor1つ分のサイズを用意する
	materialResource_ = TuboEngine::DirectXCommon::GetInstance()->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む
	//書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	//今回は白を書き込んでみる
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->enableLighting = false;
	materialData_->uvTransform = MakeIdentity4x4();


#pragma endregion


	textureIndex_ = TextureManager::GetInstance()->GetSrvIndex(textureFilePath);

	AdjustTextureSize();
}

void Sprite::Update()
{

	//textureの位置
	float left = 0.0f - anchorPoint_.x;
	float right = 1.0f - anchorPoint_.x;
	float top = 0.0f - anchorPoint_.y;
	float bottom = 1.0f - anchorPoint_.y;


	// 左右反転
	if (isFlipX_) {
		left = 1.0f - anchorPoint_.x;
		right = 0.0f - anchorPoint_.x;
	}
	else {
		left = 0.0f - anchorPoint_.x;
		right = 1.0f - anchorPoint_.x;
	}
	//上下反転
	if (isFlipY_) {
		top = 1.0f - anchorPoint_.x;
		bottom = 0.0f - anchorPoint_.x;
	}
	else {
		top = 0.0f - anchorPoint_.x;
		bottom = 1.0f - anchorPoint_.x;
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
	if (isAdjustTextureSize_) {
		AdjustTextureSize();
	}

	/*---------------------------------------
	テクスチャの位置、画像位置, 法線ベクトル, 大きさ
	---------------------------------------*/

	transform_.translate = { position_.x,position_.y,0.0f };
	transform_.rotate = { 0.0f,0.0f,rotation_ };

	vertexData_[0].position = { left,bottom,0.0f,1.0f };
	vertexData_[0].texcoord = { tex_left,tex_bottom };
	vertexData_[0].normal = { 0.0f,0.0f,-1.0f };

	vertexData_[1].position = { left,top,0.0f,1.0f };
	vertexData_[1].texcoord = { tex_left,tex_top };
	vertexData_[1].normal = { 0.0f,0.0f,-1.0f };

	vertexData_[2].position = { right,bottom,0.0f,1.0f };
	vertexData_[2].texcoord = { tex_right,tex_bottom };
	vertexData_[2].normal = { 0.0f,0.0f,-1.0f };

	vertexData_[3].position = { right,top,0.0f,1.0f };
	vertexData_[3].texcoord = { tex_right,tex_top };
	vertexData_[3].normal = { 0.0f,0.0f,-1.0f };

	transform_.scale = { size_.x,size_.y,1.0f };


	/*---------
	行列更新処理
	---------*/
	Matrix4x4 uvTransformMatrix = MakeAffineMatrix(uvTransFormMatrix_.scale, uvTransFormMatrix_.rotate, uvTransFormMatrix_.translate);

	materialData_->uvTransform = uvTransformMatrix;

	Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	Matrix4x4 projectionMatrix =
	    MakeOrthographicMatrix(0.0f, 0.0f, float(TuboEngine::WinApp::GetInstance()->GetClientWidth()), float(TuboEngine::WinApp::GetInstance()->GetClientHeight()), 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData_->WVP = worldViewProjectionMatrix;
	transformationMatrixData_->World = worldMatrix;



	commandList_ = TuboEngine::DirectXCommon::GetInstance()->GetCommandList();



}


void Sprite::Draw(){

	

	commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList_->IASetIndexBuffer(&indexBufferView_);

	commandList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

	//TransformationMatrixCBufferの設定
	commandList_->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());

	commandList_->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureFilePath_));
	//描画
	commandList_->DrawIndexedInstanced(6, 1, 0, 0, 0);

}

void Sprite::AdjustTextureSize()
{

	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureFilePath_);

	// 切り出しをテクスチャ全体にリセット
	textureLeftTop_ = {0.0f, 0.0f};
	textureSize_.x = static_cast<float>(metadata.width);
	textureSize_.y = static_cast<float>(metadata.height);
	
	size_ = textureSize_;
}

// ImGuiでSpriteの全機能をまとめて操作・確認できる関数
void Sprite::DrawImGui(const char* windowName) {
#ifdef USE_IMGUI

	const char* windowName_ = windowName;
	ImGui::Begin(windowName_);
    ImGui::Separator();
    ImGui::Text("Sprite ImGui コントロール"); // セクションタイトル
    // 位置
    TuboEngine::Math::Vector2 pos = GetPosition();
    if (ImGui::DragFloat2("Position", &pos.x, 1.0f)) {
        SetPosition(pos);
    }
    // 回転
    float rot = GetRotation();
    if (ImGui::DragFloat("Rotation", &rot, 0.01f)) {
        SetRotation(rot);
    }
    // サイズ
	TuboEngine::Math::Vector2 sz = GetSize();
    if (ImGui::DragFloat2("Size", &sz.x, 1.0f)) {
        SetSize(sz);
    }
    // アンカーポイント
	TuboEngine::Math::Vector2 anchor = GetAnchorPoint();
    if (ImGui::DragFloat2("AnchorPoint", &anchor.x, 0.01f, 0.0f, 1.0f)) {
        SetAnchorPoint(anchor);
    }
    // 左右フリップ
    bool flipX = GetFlipX();
    if (ImGui::Checkbox("FlipX", &flipX)) {
        SetFlipX(flipX);
    }
    // 上下フリップ
    bool flipY = GetFlipY();
    if (ImGui::Checkbox("FlipY", &flipY)) {
        SetFlipY(flipY);
    }
    // テクスチャ左上座標
	TuboEngine::Math::Vector2 texLT = GetTextureLeftTop();
    if (ImGui::DragFloat2("TextureLeftTop", &texLT.x, 1.0f)) {
        SetTextureLeftTop(texLT);
    }
    // テクスチャ切り出しサイズ
	TuboEngine::Math::Vector2 texSz = GetTextureSize();
    if (ImGui::DragFloat2("TextureSize", &texSz.x, 1.0f)) {
        SetTextureSize(texSz);
    }
    // テクスチャ初期サイズ調整フラグ
    bool adjust = GetIsAdjustTextureSize();
    if (ImGui::Checkbox("AdjustTextureSize", &adjust)) {
        SetGetIsAdjustTextureSize(adjust);
    }
    // 色
    Vector4 color = GetColor();
    if (ImGui::ColorEdit4("Color", &color.x)) {
        SetColor(color);
    }
    // コメント: ここでSpriteの全てのプロパティをImGuiで操作できます
	ImGui::End(); // ウィンドウ終了

	#endif // USE_IMGUI
}
