#pragma once
#include"VertexData.h"
#include"DirectXcommon.h"
#include"WinApp.h"
#include"Matrix4x4.h"
#include"Material.h"
#include"TransformationMatrix.h"
#include"Transform.h"



class SpriteCommon;
class Sprite
{
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="spriteCommon">スプライトの共通情報を保持するポインタ。</param>
	/// /// <param name="dxCommon">WinAppを保持するポインタ。</param>
	/// <param name="dxCommon">DirectXの共通情報を保持するポインタ。</param>
	void Initialize(std::string textureFilePath);

	/// <summary>
	///　更新処理
	/// </summary>
	void Update();

	/// <summary>
	///　描画処理
	/// </summary>
	void Draw();


	//getter_Pos
	const Vector2& GetPosition()const { return position; }
	//setter_Pos
	void SetPosition(const Vector2& position) { this->position = position; }


	//getter_Rotation
	const float& GetRotation()const { return rotation; }
	//setter_Rotation
	void SetRotation(const float& rotation) { this->rotation = rotation; }

	//getter_Color
	const Vector4& GetColor()const { return materialData->color;}
	//setter_Color
	void SetColor(const Vector4& color) { materialData->color = color; }


	//getter_Size
	const Vector2& GetSize()const { return size; }
	//setter_Size
	void SetSize(const Vector2& size) { this->size = size; }

	const Vector2& GetAnchorPoint()const { return anchorPoint; }
	void SetAnchorPoint(const Vector2& anchorPoint) { this->anchorPoint = anchorPoint; }

	const bool& GetFlipX()const { return isFlipX_; }
	void SetFlipX(const bool& isFlipX) { this->isFlipX_ = isFlipX; }

	const bool& GetFlipY()const { return isFlipY_; }
	void SetFlipY(const bool& isFlipY) { this->isFlipY_ = isFlipY; }

	const Vector2& GetTextureLeftTop()const { return textureLeftTop_; }
	void SetTextureLeftTop(const Vector2& textureLeftTop) { this->textureLeftTop_ = textureLeftTop; }

	const Vector2& GetTextureSize()const { return textureSize_; }
	void SetTextureSize(const Vector2& textureSize) { this->textureSize_ = textureSize; }

	const bool& GetIsAdjustTextureSize()const { return isAdjustTextureSize; }
	void SetGetIsAdjustTextureSize(const bool& isAdjustTextureSize) { this->isAdjustTextureSize = isAdjustTextureSize; }
	
	/// <summary>
	/// テクスチャから初期サイズを得る
	/// </summary>
	void AdjustTextureSize();

private:
	
	//バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr <ID3D12Resource> indexResource;

	//バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	//バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	//バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> materialResource;
	//バッファリソース内のデータを指すポインタ
	Material* materialData = nullptr;

	//バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> transformationMatrixResource;
	//バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData = nullptr;


	Transform uvTransFormMatrix{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};
	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList;


	Vector2 position = {};
	float rotation = {};
	Vector2 size = { 640.0f,360.0f };

	//テクスチャ番号
	uint32_t textureIndex = 0;

	std::string textureFilePath_;

	/*----------
	　　拡張機能
	-----------*/
	//アンカーポイント
	Vector2 anchorPoint = {};
	//左右フリップ
	bool isFlipX_ = false;
	//上下フリップ
	bool isFlipY_ = false;
	//テクスチャ左上座標
	Vector2 textureLeftTop_ = {};
	//テクスチャ切り出しサイズ
	Vector2 textureSize_ = { 100.0f,100.0f };
	//初期サイズにするフラグ
	bool isAdjustTextureSize;
};

