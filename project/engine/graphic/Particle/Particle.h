#pragma once
#include"DirectXcommon.h"
#include"WinApp.h"
#include"VertexData.h"
#include"Material.h"
#include"TransformationMatrix.h"
#include"Transform.h"
#include"ModelData.h"
#include"Camera.h"
#include"ParticleCommon.h"
#include"ModelCommon.h"
#include"ModelManager.h"
#include <random>


//Particle情報
struct ParticleInfo {
	//位置、拡大率、回転
	Transform transform;
	//速度
	Vector3 velocity;
	//カラー
	Vector4 color;
	//寿命
	float lifeTime;
	//経過時間
	float currentTime;
};

struct ParticleForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

// パーティクルグループ構造体の定義
struct ParticleGroup {
	// マテリアルデータ
	std::string materialFilePath;
	int srvIndex = 0;
	// パーティクルのリスト (std::list<ParticleStr>型)
	std::list<ParticleInfo> particleList = {};
	// インスタンシングデータ用SRVインデックス
	int instancingSrvIndex = 0;
	// インスタンシングリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource = nullptr;
	// インスタンス数
	UINT instanceCount = 0;
	// インスタンシングデータを書き込むためのポインタ
	ParticleForGPU* instancingDataPtr = nullptr;

	Vector2 textureLeftTop = { 0.0f, 0.0f }; // テクスチャ左上座標
	Vector2 textureSize = { 0.0f, 0.0f }; // テクスチャサイズを追加
};

//前方宣言
class ParticleCommon;
class ModelCommon;
class Model;
class Camera;


class Particle
{
public:


	//平行光源
	struct DirectionalLight {
		//色
		Vector4 color;
		//方向
		Vector3 direction;
		//強度
		float intensity;
	};




public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="particleCommon"></param>
	void Initialize(ParticleCommon* particleCommon);
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();
	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	void Emit(const std::string name, const Vector3& position, uint32_t count);

	void CreateParticleGroup(const std::string& name, const std::string& textureFilePath);

private:

	/// <summary>
	/// 頂点データ作成
	/// </summary>
	void CreateVertexData();

	/// <summary>
	/// 頂点バッファビューの作成
	/// </summary>
	void CreateVertexBufferView();


	void CreateMaterialData();

	/// <summary>
	/// パーティクルの作成
	/// </summary>
	/// <param name="randomEngine">ランダム</param>
	/// <param name="position">座標</param>
	/// <returns></returns>
	ParticleInfo CreateNewParticle(std::mt19937& randomEngine, const Vector3& position);



public:
	//Setter
	void SetScale(const Vector3& scale) { transform.scale = scale; }
	Vector3 GetScale() { return transform.scale; }
	void SetRotation(const Vector3& rotation) { transform.rotate = rotation; }
	Vector3 GetRotation() { return transform.rotate; }
	void SetPosition(const Vector3& position) { transform.translate = position; }
	Vector3 GetPosition() { return transform.translate; }
	//モデルのセット
	void SetModel(Model* model) {
		assert(model);
		this->model_ = model;
	}

	void SetModel(const std::string& filePath)
	{
		model_ = ModelManager::GetInstance()->FindModel(filePath);
	}

	/// <summary>
	/// カメラのセット
	/// </summary>
	/// <param name="camera"></param>
	void SetCamera(Camera* camera) {
		assert(camera);
		this->camera_ = camera;
	}
private:
	//パーティクル共通部分
	ParticleCommon* particleCommon = nullptr;
	//DirectX共通部分
	DirectXCommon* dxCommon_ = nullptr;
	//ウィンドウズアプリケーション
	WinApp* winApp_ = nullptr;

	//モデル共通部分
	ModelCommon* modelCommon_ = nullptr;
	//モデルデータ
	Model* model_ = nullptr;


	//バッファリソース内のデータを指すポインタ
	DirectionalLight* directionalLightData = nullptr;
	//コマンドリスト
	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList;

	//3Dオブジェクトの座標
	Transform transform;
	//カメラ座標
	Transform cameraTransform;
	//カメラ
	Camera* camera_;


	// パーティクルグループ
	std::unordered_map<std::string, ParticleGroup> particleGroups;

	//---------------------------------------
	// モデルデータ
	ModelData modelData_;

	//---------------------------------------
	// 頂点データ
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	// バッファリソースの使い道を指すポインタ
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData_ = nullptr;

	//---------------------------------------
	// マテリアルデータ
	Microsoft::WRL::ComPtr<ID3D12Resource> materialBuffer_;
	// バッファリソース内のデータを指すポインタ
	Material* materialData_ = nullptr;

	//---------------------------------------
	// インスタンシングバッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingBuffer_;

	//---------------------------------------
	// 乱数生成器の初期化
	std::random_device seedGenerator_;
	std::mt19937 randomEngine_;

	//---------------------------------------
	// その他
	// カメラ目線を使用するかどうか
	bool isUsedBillboard = true;
	//最大インスタンス数
	static const uint32_t kNumMaxInstance = 128;
	//
	const float kDeltaTime = 1.0f / 60.0f;
	// 乱数範囲の調整用
	struct RangeForRandom {
		float min;
		float max;
	};

	// パーティクルの設定
	RangeForRandom translateRange_ = { 0.0f, 0.0f };
	RangeForRandom colorRange_ = { 1.0f, 1.0f };
	RangeForRandom lifetimeRange_ = { 1.0f, 3.0f };
	RangeForRandom velocityRange_ = { -1.1f, 1.1f };


	//パーティクルのテクスチャサイズ
	Vector2 customTextureSize = { 100.0f, 100.0f };
};

