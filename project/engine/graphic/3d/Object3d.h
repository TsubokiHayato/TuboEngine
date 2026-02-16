#pragma once
#include"DirectXcommon.h"
#include"WinApp.h"

#include"VertexData.h"
#include"Material.h"
#include"TransformationMatrix.h"
#include"Transform.h"
#include"ModelData.h"
#include"BlendMode.h"
#include"CameraForGPU.h"
#include"SkyBox.h"
//前方宣言
class Object3dCommon;
class ModelCommon;
class Model;
class Camera;

//平行光源
struct DirectionalLight {
	//色
	TuboEngine::Math::Vector4 color;
	//方向
	TuboEngine::Math::Vector3 direction;
	//強度
	float intensity;
};

//PointLight
struct PointLight
{
	//色
	TuboEngine::Math::Vector4 color;
	//位置
	TuboEngine::Math::Vector3 position;
	//輝度
	float intensity;
};

struct SpotLight
{
	//色
	TuboEngine::Math::Vector4 color;
	//位置
	TuboEngine::Math::Vector3 position;
	//輝度
	float intensity;
	//方向
	TuboEngine::Math::Vector3 direction;
	//ライトの届く最大距離
	float distance;
	//減衰率
	float decay;
	//スポットライトの余弦
	float cosAngle;
	//
	float padding[2];
};

struct LightType {

	//0 : 平行光源
	//1 : Phong反射モデル
	//2 : Blinn-Phong反射モデル
	//3 : PointLight
	//4 : SpotLight
	int type;

};


/**
 * @brief 3Dモデルをワールド空間に配置して描画するための描画オブジェクト。
 *
 * @details
 * - Transform（位置/回転/拡縮）を保持し、更新時にGPU用の行列定数（`TransformationMatrix`）へ反映します。
 * - `Model` と `Camera` を参照し、描画コマンドを発行します。
 * - 平行光源/ポイントライト/スポットライト等のライトパラメータを保持し、シェーダへ渡します。
 *
 * @note 本クラスは「ゲーム固有の振る舞い」を持たず、描画に必要な状態の保持と反映が責務です。
 */
class Object3d
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="object3dCommon"></param>
	/**
	 * @brief 3Dオブジェクトを初期化し、指定されたモデルをロード/設定します。
	 * @param modelFileNamePath 読み込むモデルファイルパス（例: `"xxx.obj"`）。
	 */
	void Initialize(std::string modelFileNamePath);

	/// <summary>
	/// 更新処理
	/// </summary>
	/**
	 * @brief Transform/カメラ/ライト等の現在値をGPUリソースへ反映します。
	 */
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	/**
	 * @brief 現在の状態で描画コマンドを発行します。
	 */
	void Draw();

	/// <summary>
	/// ImGuiで全機能をまとめて操作・確認する関数
	/// </summary>
	/**
	 * @brief ImGui上にデバッグUIを表示します。
	 * @param windowName ImGuiウィンドウ名。
	 */
	void DrawImGui(const char* windowName);

	

public:

	//Setter
	/** @brief ワールド拡縮を設定します。 @param scale スケール。 */
	void SetScale(const TuboEngine::Math::Vector3& scale) { transform_.scale = scale; }
	/** @brief ワールド回転を設定します。 @param rotation 回転（ラジアン想定）。 */
	void SetRotation(const TuboEngine::Math::Vector3& rotation) { transform_.rotate = rotation; }
	/** @brief ワールド位置を設定します。 @param position 位置。 */
	void SetPosition(const TuboEngine::Math::Vector3& position) { transform_.translate = position; }

	///-------------------------------------------------------------------------------------------------
	/// Light
	//平行光源
	/** @brief 平行光源の色を設定します。 @param color RGBA。 */
	void SetLightColor(const TuboEngine::Math::Vector4& color) { directionalLightData_->color = color; }
	/** @brief 平行光源の向きを設定します。 @param direction 方向ベクトル。 */
	void SetLightDirection(const TuboEngine::Math::Vector3& direction) { directionalLightData_->direction = direction; }
	/** @brief 平行光源の強度を設定します。 @param intensity 強度。 */
	void SetLightIntensity(float intensity) { directionalLightData_->intensity = intensity; }
	/**
	 * @brief 平行光源の鏡面反射の鋭さ（シェーディング用）を設定します。
	 * @param shininess シャイニネス。
	 */
	void SetLightShininess(float shininess);
	//ポイントライト
	/** @brief ポイントライト位置を設定します。 @param position 位置。 */
	void SetPointLightPosition(const TuboEngine::Math::Vector3& position) { pointLightData_->position = position; }
	/** @brief ポイントライトの色を設定します。 @param color RGBA。 */
	void SetPointLightColor(const TuboEngine::Math::Vector4& color) { pointLightData_->color = color; }
	/** @brief ポイントライトの強度を設定します。 @param intensity 強度。 */
	void SetPointLightIntensity(float intensity) { pointLightData_->intensity = intensity; }
	//スポットライト
	/** @brief スポットライトの色を設定します。 @param color RGBA。 */
	void SetSpotLightColor(const TuboEngine::Math::Vector4& color) { spotLightData_->color = color; }
	/** @brief スポットライトの位置を設定します。 @param position 位置。 */
	void SetSpotLightPosition(const TuboEngine::Math::Vector3& position) { spotLightData_->position = position; }
	/** @brief スポットライトの向きを設定します。 @param direction 方向ベクトル。 */
	void SetSpotLightDirection(const TuboEngine::Math::Vector3& direction) { spotLightData_->direction = direction; }
	/** @brief スポットライトの強度を設定します。 @param intensity 強度。 */
	void SetSpotLightIntensity(float intensity) { spotLightData_->intensity = intensity; }
	/** @brief スポットライトの到達距離を設定します。 @param distance 最大距離。 */
	void SetSpotLightDistance(float distance) { spotLightData_->distance = distance; }
	/** @brief スポットライトの減衰率を設定します。 @param decay 減衰率。 */
	void SetSpotLightDecay(float decay) { spotLightData_->decay = decay; }
	/** @brief スポットライトの開き角（余弦）を設定します。 @param cosAngle cos(角度)。 */
	void SetSpotLightCosAngle(float cosAngle) { spotLightData_->cosAngle = cosAngle; }

	/**
	 * @brief 使用するライト種別を設定します。
	 * @param type ライト種別（実装依存）。範囲外は0に丸めます。
	 */
	void SetLightType(int type) {
		if (type < 0 || type > 5) {
			type = 0;
		}
		lightTypeData_->type = type;
	}

	/**
	 * @brief 参照する`Model`を設定します。
	 * @param model モデルポインタ（`nullptr`不可）。
	 */
	void SetModel(Model* model) {
		assert(model);
		this->model_ = model;
	}
	/**
	 * @brief ファイルパスからモデルを設定します。
	 * @param filePath モデルファイルパス。
	 */
	void SetModel(const std::string& filePath);
	/**
	 * @brief 描画に使用するカメラを設定します。
	 * @param camera カメラ。
	 */
	void SetCamera(Camera* camera) { this->camera_ = camera; }
	/**
	 * @brief 現在設定されているカメラを取得します。
	 * @return カメラポインタ。
	 */
	Camera* GetCamera() const { return camera_; }

	/**
	 * @brief モデルカラー（マテリアル色）を設定します。
	 * @param color RGBA。
	 */
	void SetModelColor(const TuboEngine::Math::Vector4& color);

	//Getter
	/** @brief ワールド拡縮を取得します。 @return スケール。 */
	TuboEngine::Math::Vector3 GetScale() const { return transform_.scale; }
	/** @brief ワールド回転を取得します。 @return 回転。 */
	TuboEngine::Math::Vector3 GetRotation() const { return transform_.rotate; }
	/** @brief ワールド位置を取得します。 @return 位置。 */
	TuboEngine::Math::Vector3 GetPosition() const { return transform_.translate; }
	//モデルの色
	/** @brief モデルカラーを取得します。 @return RGBA。 */
	Vector4 GetModelColor();


	//-------------------------------------------------------------------------------------------------
	//Material


	///-------------------------------------------------------------------------------------------------
	/// Light
	//平行光源
	/**
	 * @brief 平行光源の色を取得します。
	 * @return RGBA。
	 */
	TuboEngine::Math::Vector4 GetLightColor() { return directionalLightData_->color; }
	/**
	 * @brief 平行光源の向きを取得します。
	 * @return 方向ベクトル。
	 */
	TuboEngine::Math::Vector3 GetLightDirection() { return directionalLightData_->direction; }
	/**
	 * @brief 平行光源の強度を取得します。
	 * @return 強度。
	 */
	float GetLightIntensity() { return directionalLightData_->intensity; }
	/**
	 * @brief 現在設定されているライト種別を取得します。
	 * @return ライト種別。
	 */
	int GetLightType() { return lightTypeData_->type; }
	/**
	 * @brief 平行光源の鏡面反射の鋭さを取得します。
	 * @return シャイニネス。
	 */
	float GetLightShininess();
	//ポイントライト
	/**
	 * @brief ポイントライト位置を取得します。
	 * @return 位置。
	 */
	TuboEngine::Math::Vector3 GetPointLightPosition() { return pointLightData_->position; }
	/**
	 * @brief ポイントライトの色を取得します。
	 * @return RGBA。
	 */
	TuboEngine::Math::Vector4 GetPointLightColor() { return pointLightData_->color; }
	/**
	 * @brief ポイントライトの強度を取得します。
	 * @return 強度。
	 */
	float GetPointLightIntensity() { return pointLightData_->intensity; }
	//スポットライト
	/**
	 * @brief スポットライトの色を取得します。
	 * @return RGBA。
	 */
	void GetSpotLightColor(TuboEngine::Math::Vector4& color) { color = spotLightData_->color; }
	/**
	 * @brief スポットライトの位置を取得します。
	 * @return 位置。
	 */
	void GetSpotLightPosition(TuboEngine::Math::Vector3& position) { position = spotLightData_->position; }
	/**
	 * @brief スポットライトの向きを取得します。
	 * @return 方向ベクトル。
	 */
	void GetSpotLightDirection(TuboEngine::Math::Vector3& direction) { direction = spotLightData_->direction; }
	/**
	 * @brief スポットライトの強度を取得します。
	 * @return 強度。
	 */
	float GetSpotLightIntensity() { return spotLightData_->intensity; }
	/**
	 * @brief スポットライトの到達距離を取得します。
	 * @return 最大距離。
	 */
	float GetSpotLightDistance() { return spotLightData_->distance; }
	/**
	 * @brief スポットライトの減衰率を取得します。
	 * @return 減衰率。
	 */
	float GetSpotLightDecay() { return spotLightData_->decay; }
	/**
	 * @brief スポットライトの開き角（余弦）を取得します。
	 * @return cos(角度)。
	 */
	float GetSpotLightCosAngle() { return spotLightData_->cosAngle; }


	/**
	 * @brief キューブマップ（環境マップ）テクスチャのパスを設定します。
	 * @param filePath キューブマップファイルパス。
	 */
	void SetCubeMapFilePath(const std::string& filePath) {
		cubeMapFilePath_ = filePath;
	}
	
private:

	//-------------------------------------------------------------------
	//		メンバ変数

	//モデル共通部分
	ModelCommon* modelCommon_ = nullptr;
	//モデルデータ
	Model* model_ = nullptr;
	//カメラ
	Camera* camera_ = nullptr;

	//座標のバッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> transformMatrixResource_;
	//座標のバッファリソース内のデータを指すポインタ
	TransformationMatrix* transformMatrixData_ = nullptr;

	//平行光源のバッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResource_;
	//バッファリソース内のデータを指すポインタ
	DirectionalLight* directionalLightData_ = nullptr;

	//ポイントライトのバッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> pointLightResource_;
	//バッファリソース内のデータを指すポインタ
	PointLight* pointLightData_ = nullptr;

	//スポットライトのバッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> spotLightResource_;
	//バッファリソース内のデータを指すポインタ
	SpotLight* spotLightData_ = nullptr;


	//コマンドリスト
	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList_;

	//カメラ座標のバッファリソース
	Microsoft::WRL::ComPtr <ID3D12Resource> cameraForGPUResource_ = nullptr;
	//カメラ座標のバッファリソース内のデータを指すポインタ
	CameraForGPU* cameraForGPUData_ = nullptr;

	//ライトの種類
	Microsoft::WRL::ComPtr <ID3D12Resource> lightTypeResource_ = nullptr;
	//ライトの種類のバッファリソース内のデータを指すポインタ
	LightType* lightTypeData_ = nullptr;

	//3Dオブジェクトの座標
	Transform transform_;
	//カメラ座標
	Transform cameraTransform_;

	// キューブマップのSRVハンドル
	//デフォルト : ロストック・ラージ空港の4Kキューブマップ
	std::string cubeMapFilePath_ = "rostock_laage_airport_4k.dds";

};

