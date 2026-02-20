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

	/// <summary>
	/// ImGuiで全機能をまとめて操作・確認する関数
	/// </summary>
	void DrawImGui(const char* windowName);

	// getter/setter
	const TuboEngine::Math::Vector2& GetPosition() const { return position_; }
	void SetPosition(const TuboEngine::Math::Vector2& position) { position_ = position; }

	const float& GetRotation()const { return rotation_; }
	void SetRotation(const float& rotation) { rotation_ = rotation; }

	const TuboEngine::Math::Vector4& GetColor() const { return materialData_->color; }
	void SetColor(const TuboEngine::Math::Vector4& color) { materialData_->color = color; }

	const TuboEngine::Math::Vector2& GetSize() const { return size_; }
	void SetSize(const TuboEngine::Math::Vector2& size) { size_ = size; }

	const TuboEngine::Math::Vector2& GetAnchorPoint() const { return anchorPoint_; }
	void SetAnchorPoint(const TuboEngine::Math::Vector2& anchorPoint) { anchorPoint_ = anchorPoint; }

	const bool& GetFlipX()const { return isFlipX_; }
	void SetFlipX(const bool& isFlipX) { isFlipX_ = isFlipX; }

	const bool& GetFlipY()const { return isFlipY_; }
	void SetFlipY(const bool& isFlipY) { isFlipY_ = isFlipY; }

	const TuboEngine::Math::Vector2& GetTextureLeftTop() const { return textureLeftTop_; }
	void SetTextureLeftTop(const TuboEngine::Math::Vector2& textureLeftTop) { textureLeftTop_ = textureLeftTop; }

	const TuboEngine::Math::Vector2& GetTextureSize() const { return textureSize_; }
	void SetTextureSize(const TuboEngine::Math::Vector2& textureSize) { textureSize_ = textureSize; }

	const bool& GetIsAdjustTextureSize()const { return isAdjustTextureSize_; }
	void SetGetIsAdjustTextureSize(const bool& isAdjustTextureSize) { isAdjustTextureSize_ = isAdjustTextureSize; }
	
	/// <summary>
	/// テクスチャから初期サイズを得る
	/// </summary>
	void AdjustTextureSize();

	/// <summary>
	/// テクスチャを差し替える（SRVだけ切り替え）。
	/// </summary>
	void SetTexture(const std::string& textureFilePath) { textureFilePath_ = textureFilePath; }
	const std::string& GetTexture() const { return textureFilePath_; }

private:
	//バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr <ID3D12Resource> indexResource_;

	//バッファリソース内のデータを指すポインタ
	VertexData* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;
	//バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;

	//バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> materialResource_;
	//バッファリソース内のデータを指すポインタ
	Material* materialData_ = nullptr;

	//バッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> transformationMatrixResource_;
	//バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData_ = nullptr;


	Transform uvTransFormMatrix_{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};
	Transform transform_{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList_;


	TuboEngine::Math::Vector2 position_ = {};
	float rotation_ = {};
	TuboEngine::Math::Vector2 size_ = {640.0f, 360.0f};

	//テクスチャ番号
	uint32_t textureIndex_ = 0;

	std::string textureFilePath_;

	/*----------
	　　 拡張機能
	-----------*/
	//アンカーポイント
	TuboEngine::Math::Vector2 anchorPoint_ = {};
	//左右フリップ
	bool isFlipX_ = false;
	//上下フリップ
	bool isFlipY_ = false;
	//テクスチャ左上座標
	TuboEngine::Math::Vector2 textureLeftTop_ = {};
	//テクスチャ切り出しサイズ
	TuboEngine::Math::Vector2 textureSize_ = {100.0f, 100.0f};
	//初期サイズにするフラグ
	bool isAdjustTextureSize_ = false;
};

