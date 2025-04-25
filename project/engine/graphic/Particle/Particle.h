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

// 前方宣言
class ParticleCommon;
class ModelCommon;
class Model;
class Camera;


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

// パーティクルのタイプ
	enum class ParticleType
	{
		None, // なし
		Primitive, // プリミティブ
		Ring, // リング
		Cylinder, // 円柱
	};

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
	void Initialize(ParticleCommon* particleCommon, ParticleType particleType);

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
	/// <param name="velocity">速度</param>
	/// <param name="color">カラー</param>
	/// <param name="lifeTime">寿命</param>
	/// <param name="currentTime">経過時間</param>
	/// <param name="count">生成数</param>
	void Emit(const std::string name, const Transform& transform, Vector3 velocity, Vector4 color, float lifeTime, float currentTime, uint32_t count);

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
	/// 頂点データの作成(リング)
	/// </summary>
	void CreateVertexDataForRing();

	/// <summary>
	/// 頂点データの作成(円柱)
	/// </summary>
	void CreateVertexDataForCylinder();



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
	///<param name="transform">エミッターのトランスフォーム</param>
	/// <param name="velocity">速度</param>
	/// <param name="color">カラー</param>
	/// <param name="lifeTime">寿命</param>
	/// <param name="currentTime">経過時間</param>
	/// <returns>新しいパーティクル情報</returns>
	ParticleInfo CreateNewParticle(std::mt19937& randomEngine, const Transform& transform, Vector3 velocity, Vector4 color, float lifeTime, float currentTime);

	/// <summary>
	/// 新しいパーティクルの作成(リング)
	/// </summary>
	/// <param name="randomEngine">ランダムエンジン</param>
	///<param name="transform">エミッターのトランスフォーム</param>
	/// <param name="velocity">速度</param>
	/// <param name="color">カラー</param>
	/// <param name="lifeTime">寿命</param>
	/// <param name="currentTime">経過時間</param>
	/// <returns>新しいパーティクル情報</returns>
	ParticleInfo CreateNewParticleForRing(std::mt19937& randomEngine, const Transform& transform, Vector3 velocity, Vector4 color, float lifeTime, float currentTime);

	/// <summary>
	/// 新しいパーティクルの作成(円柱)
	/// </summary>
	/// <param name="randomEngine">ランダムエンジン</param>
	///<param name="transform">エミッターのトランスフォーム</param>
	/// <param name="velocity">速度</param>
	/// <param name="color">カラー</param>
	/// <param name="lifeTime">寿命</param>
	/// <param name="currentTime">経過時間</param>
	/// <returns>新しいパーティクル情報</returns>
	ParticleInfo CreateNewParticleForCylinder(std::mt19937& randomEngine, const Transform& transform, Vector3 velocity, Vector4 color, float lifeTime, float currentTime);

public:

#pragma region Getters & Setters
	///----------------------------------------------------------------------------------------------------------------------
	// Setter
	///----------------------------------------------------------------------------------------------------------------------

#pragma region Setter

#pragma region 位置
	/// <summary>
	/// 位置Xの範囲を設定
	/// <param name="min">最小値</param>
	/// <param name="max">最大値</param>
	/// </summary>
	void SetDistTranslateX(float min, float max) { translateRange_.x.min = min; translateRange_.x.max = max; }

	/// <summary>
	/// 位置Yの範囲を設定
	/// <param name="min">最小値</param>
	/// <param name="max">最大値</param>
	/// </summary>
	void SetDistTranslateY(float min, float max) { translateRange_.y.min = min; translateRange_.y.max = max; }

	/// <summary>
	/// 位置Zの範囲を設定
	/// <param name="min">最小値</param>
	/// <param name="max">最大値</param>
	/// </summary>
	void SetDistTranslateZ(float min, float max) { translateRange_.z.min = min; translateRange_.z.max = max; }

	/// <summary>
	/// 位置の範囲を一括設定
	/// </summary>
	/// <param name="minX"> Xの最小値</param>
	/// <param name="maxX"> 
	/// <param name="minY"> 
	/// <param name="maxY"> 
	/// <param name="minZ"> 
	/// <param name="maxZ"> 
	void SetDistTranslate(float minX, float maxX, float minY, float maxY, float minZ, float maxZ) {
		translateRange_.x.min = minX; translateRange_.x.max = maxX;
		translateRange_.y.min = minY; translateRange_.y.max = maxY;
		translateRange_.z.min = minZ; translateRange_.z.max = maxZ;
	}

#pragma endregion 位置

	//// 回転の範囲を設定
	//void SetDistRotateX(float min, float max) { rotateRange_.x.min = min; rotateRange_.x.max = max; }
	//void SetDistRotateY(float min, float max) { rotateRange_.y.min = min; rotateRange_.y.max = max; }
	//void SetDistRotateZ(float min, float max) { rotateRange_.z.min = min; rotateRange_.z.max = max; }

	//// 拡大の範囲を設定
	//void SetDistScaleX(float min, float max) { scaleRange_.x.min = min; scaleRange_.x.max = max; }
	//void SetDistScaleY(float min, float max) { scaleRange_.y.min = min; scaleRange_.y.max = max; }
	//void SetDistScaleZ(float min, float max) { scaleRange_.z.min = min; scaleRange_.z.max = max; }

	//// 速度の範囲を設定
	//void SetDistVelocityX(float min, float max) { velocityRange_.x.min = min; velocityRange_.x.max = max; }
	//void SetDistVelocityY(float min, float max) { velocityRange_.y.min = min; velocityRange_.y.max = max; }
	//void SetDistVelocityZ(float min, float max) { velocityRange_.z.min = min; velocityRange_.z.max = max; }

	//
	//

	//// 回転の範囲を一括設定
	//void SetDistRotate(float minX, float maxX, float minY, float maxY, float minZ, float maxZ) {
	//	rotateRange_.x.min = minX; rotateRange_.x.max = maxX;
	//	rotateRange_.y.min = minY; rotateRange_.y.max = maxY;
	//	rotateRange_.z.min = minZ; rotateRange_.z.max = maxZ;
	//}

	//// 拡大の範囲を一括設定
	//void SetDistScale(float minX, float maxX, float minY, float maxY, float minZ, float maxZ) {
	//	scaleRange_.x.min = minX; scaleRange_.x.max = maxX;
	//	scaleRange_.y.min = minY; scaleRange_.y.max = maxY;
	//	scaleRange_.z.min = minZ; scaleRange_.z.max = maxZ;
	//}

	//// 速度の範囲を一括設定
	//void SetDistVelocity(float minX, float maxX, float minY, float maxY, float minZ, float maxZ) {
	//	velocityRange_.x.min = minX; velocityRange_.x.max = maxX;
	//	velocityRange_.y.min = minY; velocityRange_.y.max = maxY;
	//	velocityRange_.z.min = minZ; velocityRange_.z.max = maxZ;
	//}


	//void SetDistRed(float min, float max) { colorRange_.x.min = min; colorRange_.x.max = max; }
	//void SetDistGreen(float min, float max) { colorRange_.y.min = min; colorRange_.y.max = max; }
	//void SetDistBlue(float min, float max) { colorRange_.z.min = min; colorRange_.z.max = max; }
	//void SetDistAlpha(float min, float max) { colorRange_.w.min = min; colorRange_.w.max = max; }

	//void SetDistColor(float minR, float maxR, float minG, float maxG, float minB, float maxB, float minA, float maxA) {
	//	colorRange_.x.min = minR; colorRange_.x.max = maxR;
	//	colorRange_.y.min = minG; colorRange_.y.max = maxG;
	//	colorRange_.z.min = minB; colorRange_.z.max = maxB;
	//	colorRange_.w.min = minA; colorRange_.w.max = maxA;
	//}

	// 寿命の範囲を一括設定
	void SetDistLifeTime(float min, float max) { lifetimeRange_.min = min; lifetimeRange_.max = max; }
	// 経過時間の範囲を一括設定
	void SetDistCurrentTime(float min, float max) { currentTimeRange_.min = min; currentTimeRange_.max = max; }

	// ビルボード(常にカメラは目線)を有効化するかどうかを設定
	void SetIsBillBoard(bool isBillBoard) { this->isBillBoard = isBillBoard; }

	void SetTransform(const Transform& transform) { this->transform = transform; } // トランスフォームを設定

	void SetCamera(Camera* camera) { assert(camera); this->camera_ = camera; } // カメラを設定

#pragma endregion Setter
	///----------------------------------------------------------------------------------------------------------------------
	// Getter
	///----------------------------------------------------------------------------------------------------------------------

	Vector3 GetPosition() { return transform.translate; } // 位置を取得
	Vector3 GetRotation() { return transform.rotate; } // 回転を取得
	Vector3 GetScale() { return transform.scale; } // スケールを取得


#pragma region Getter




#pragma endregion Getter
	

#pragma endregion Getters & Setters
private:



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

	struct RandomRange3D
	{
		RandomRange x; // X軸の乱数範囲
		RandomRange y; // Y軸の乱数範囲
		RandomRange z; // Z軸の乱数範囲
	};

	struct RandomRange4D
	{
		RandomRange x; // X軸の乱数範囲
		RandomRange y; // Y軸の乱数範囲
		RandomRange z; // Z軸の乱数範囲
		RandomRange w; // W軸の乱数範囲
	};

	RandomRange3D translateRange_ = {}; // 位置の乱数範囲
	RandomRange3D rotateRange_ = {}; // 回転の乱数範囲
	RandomRange3D scaleRange_ = {(1.0f,1.0f),(1.0f,1.0f),(1.0f,1.0f),}; // 拡大の乱数範囲
	RandomRange3D velocityRange_ = {}; // 速度の乱数範囲
	RandomRange4D colorRange_ = {}; // カラーの乱数範囲
	RandomRange lifetimeRange_ = {}; // 寿命の乱数範囲
	RandomRange currentTimeRange_ = {}; // 経過時間の乱数範囲

	Vector2 customTextureSize = { 100.0f, 100.0f }; // パーティクルのテクスチャサイズ


	////------------------


	

};
