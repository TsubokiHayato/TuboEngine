#pragma once
#include"Vector2.h"
#include"Vector3.h"
#include"Vector4.h"
#include"DirectXcommon.h"
#include"WinApp.h"
#include"Matrix4x4.h"
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};


struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};


struct  TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};


struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};



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
	void Initialize(SpriteCommon* spriteCommon,WinApp* winApp, DirectXCommon* dxCommon);

	/// <summary>
	///　更新処理
	/// </summary>
	void Update();

	/// <summary>
	///　描画処理
	/// </summary>
	void Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU);


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



private:
	SpriteCommon* spriteCommon = nullptr;
	DirectXCommon* dxCommon_ = nullptr;
	WinApp* winApp_ = nullptr;


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


};

