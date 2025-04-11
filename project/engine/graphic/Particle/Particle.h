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

// パーティクル情報
struct ParticleInfo
{
	Transform transform; // 位置、拡大率、回転
	Vector3 velocity; // 速度
	Vector4 color; // カラー
	float lifeTime; // 寿命
	float currentTime; // 経過時間
};

struct ParticleForGPU
{
	Matrix4x4 WVP; // ワールド・ビュー・プロジェクション行列
	Matrix4x4 World; // ワールド行列
	Vector4 color; // カラー
};

// パーティクルグループ構造体
struct ParticleGroup
{
	std::string materialFilePath; // マテリアルデータのファイルパス
	int srvIndex = 0; // シェーダーリソースビューのインデックス
	std::list<ParticleInfo> particleList = {}; // パーティクルのリスト
	int instancingSrvIndex = 0; // インスタンシングデータ用SRVインデックス
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource = nullptr; // インスタンシングリソース
	UINT instanceCount = 0; // インスタンス数
	ParticleForGPU* instancingDataPtr = nullptr; // インスタンシングデータのポインタ

	Vector2 textureLeftTop = { 0.0f, 0.0f }; // テクスチャの左上座標
	Vector2 textureSize = { 0.0f, 0.0f }; // テクスチャのサイズ
};

// 前方宣言
class ParticleCommon;
class ModelCommon;
class Model;
class Camera;

class Particle
{
public:
	// 平行光源
	struct DirectionalLight
	{
		Vector4 color; // 色
		Vector3 direction; // 方向
		float intensity; // 強度
	};

public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="particleCommon">パーティクル共通部分</param>
	void Initialize(ParticleCommon* particleCommon);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// パーティクルの生成
	/// </summary>
	/// <param name="name">パーティクル名</param>
	/// <param name="position">生成位置</param>
	/// <param name="count">生成数</param>
	void Emit(const std::string name, const Transform& transform, uint32_t count);

	/// <summary>
	/// パーティクルグループの作成
	/// </summary>
	/// <param name="name">グループ名</param>
	/// <param name="textureFilePath">テクスチャファイルパス</param>
	void CreateParticleGroup(const std::string& name, const std::string& textureFilePath);

private:
	/// <summary>
	/// 頂点データの作成
	/// </summary>
	void CreateVertexData();

	/// <summary>
	/// 頂点バッファビューの作成
	/// </summary>
	void CreateVertexBufferView();

	/// <summary>
	/// マテリアルデータの作成
	/// </summary>
	void CreateMaterialData();

	/// <summary>
	/// 新しいパーティクルの作成
	/// </summary>
	/// <param name="randomEngine">ランダムエンジン</param>
	/// <param name="position">生成位置</param>
	/// <returns>新しいパーティクル情報</returns>
	ParticleInfo CreateNewParticle(std::mt19937& randomEngine, const Transform& transform);

public:
	// Setter
	void SetScale(const Vector3& scale) { transform.scale = scale; }
	Vector3 GetScale() { return transform.scale; }
	void SetRotation(const Vector3& rotation) { transform.rotate = rotation; }
	Vector3 GetRotation() { return transform.rotate; }
	void SetPosition(const Vector3& position) { transform.translate = position; }
	Vector3 GetPosition() { return transform.translate; }

	/// <summary>
	/// モデルのセット
	/// </summary>
	/// <param name="model">モデルデータ</param>
	void SetModel(Model* model) {
		assert(model);
		this->model_ = model;
	}

	/// <summary>
	/// モデルのセット
	/// </summary>
	/// <param name="filePath">モデルファイルパス</param>
	void SetModel(const std::string& filePath) {
		model_ = ModelManager::GetInstance()->FindModel(filePath);
	}

	/// <summary>
	/// カメラのセット
	/// </summary>
	/// <param name="camera">カメラデータ</param>
	void SetCamera(Camera* camera) {
		assert(camera);
		this->camera_ = camera;
	}

private:
	ParticleCommon* particleCommon = nullptr; // パーティクル共通部分
	DirectXCommon* dxCommon_ = nullptr; // DirectX共通部分
	WinApp* winApp_ = nullptr; // ウィンドウズアプリケーション

	ModelCommon* modelCommon_ = nullptr; // モデル共通部分
	Model* model_ = nullptr; // モデルデータ

	DirectionalLight* directionalLightData = nullptr; // 平行光源データ
	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList; // コマンドリスト

	Transform transform; // 3Dオブジェクトの座標
	Transform cameraTransform; // カメラ座標
	Camera* camera_ = nullptr; // カメラ

	std::unordered_map<std::string, ParticleGroup> particleGroups; // パーティクルグループ

	ModelData modelData_; // モデルデータ

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_; // 頂点バッファリソース
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_; // 頂点バッファビュー
	VertexData* vertexData_ = nullptr; // 頂点データ

	Microsoft::WRL::ComPtr<ID3D12Resource> materialBuffer_; // マテリアルバッファリソース
	Material* materialData_ = nullptr; // マテリアルデータ

	Microsoft::WRL::ComPtr<ID3D12Resource> instancingBuffer_; // インスタンシングバッファ

	std::random_device seedGenerator_; // 乱数生成器
	std::mt19937 randomEngine_; // 乱数エンジン

	bool isBillBoard = true; // カメラ目線を使用するかどうか
	static const uint32_t kNumMaxInstance = 128; // 最大インスタンス数
	const float kDeltaTime = 1.0f / 60.0f; // デルタタイム

	struct RandomRange
	{
		float min; // 最小値
		float max; // 最大値
	};

	RandomRange translateRange_ = { 0.0f, 0.0f }; // 位置の乱数範囲
	RandomRange rotateXRange_ = { 0.0f,5.0f };//
	RandomRange rotateYRange_ = { 0.0f,5.0f };
	RandomRange rotateZRange_ = { 0.0f,5.0f };
	RandomRange colorRange_ = { 1.0f, 1.0f }; // カラーの乱数範囲
	RandomRange lifetimeRange_ = { 1.0f, 3.0f }; // 寿命の乱数範囲
	RandomRange velocityRange_ = { -1.1f, 1.1f }; // 速度の乱数範囲

	Vector2 customTextureSize = { 100.0f, 100.0f }; // パーティクルのテクスチャサイズ
};
