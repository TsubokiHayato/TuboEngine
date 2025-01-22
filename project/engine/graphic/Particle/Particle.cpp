#include "Particle.h"
#include "Camera.h"

#include <fstream>
#include <sstream>
#include <cmath>
#include "Vector3.h"
#include"MT_Matrix.h"
#include "TextureManager.h"
#include "ParticleCommon.h"
#include <numbers>

/// <summary>
/// 初期化処理
/// </summary>
/// <param name="particleSetup">パーティクル共通部分</param>
void Particle::Initialize(ParticleCommon* particleSetup) {
	// 引数からSetupを受け取る
	this->particleCommon = particleSetup;
	dxCommon_ = particleSetup->GetDxCommon();
	winApp_ = particleSetup->GetWinApp();

	// RandomEngineの初期化
	randomEngine_.seed(std::random_device()());

	// 頂点データの作成
	CreateVertexData();
	// 頂点バッファビューの作成
	CreateVertexBufferView();

	// 書き込むためのアドレスを取得
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	// 頂点データをリソースにコピー
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());

	camera_ = new Camera;
	camera_->SetTranslate({ 0.0f,0.0f,-5.0f });
	camera_->setRotation({ 0.0f,0.0f,0.0f });
	camera_->setScale({ 1.0f,1.0f,1.0f });
}

/// <summary>
/// 更新処理
/// </summary>
void Particle::Update() {
	camera_->Update();
	// カメラ行列の取得
	Matrix4x4 cameraMatrix = MakeAffineMatrix(camera_->GetScale(),
		camera_->GetRotation(), camera_->GetTranslate());
	// ビュー行列の取得
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);
	// プロジェクション行列の取得
	Matrix4x4 projectionMatrix = MakePerspectiveMatrix(0.45f, float(winApp_->kClientWidth) / float(winApp_->kClientHeight), 0.1f, 100.0f);
	// ビュープロジェクション行列の取得
	Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);

	// ビルボード行列の取得
	Matrix4x4 backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
	Matrix4x4 billboardMatrix{};
	if (isBillBoard) {
		billboardMatrix = Multiply(backToFrontMatrix, cameraMatrix);
		// 平行移動成分は無視
		billboardMatrix.m[3][0] = 0.0f;
		billboardMatrix.m[3][1] = 0.0f;
		billboardMatrix.m[3][2] = 0.0f;
	}
	else {
		billboardMatrix = MakeIdentity4x4();
	}

	// スケール調整用の倍率を設定
	constexpr float scaleMultiplier = 0.01f; // 必要に応じて調整

	// パーティクルの更新
	for (auto& group : particleGroups) {
		// テクスチャサイズの取得
		Vector2 textureSize = group.second.textureSize;
		// インスタンス数の初期化
		for (auto it = group.second.particleList.begin(); it != group.second.particleList.end();) {
			// パーティクルの参照
			ParticleInfo& particle = *it;
			// パーティクルの寿命が尽きた場合は削除
			if (particle.lifeTime <= particle.currentTime) {
				it = group.second.particleList.erase(it);
				continue;
			}
			// スケールをテクスチャサイズに基づいて調整
			particle.transform.scale.x = textureSize.x * scaleMultiplier;
			particle.transform.scale.y = textureSize.y * scaleMultiplier;
			// 位置の更新
			particle.transform.translate = particle.transform.translate + (particle.velocity * kDeltaTime);

			// 経過時間を更新
			particle.currentTime += kDeltaTime;
			// ワールド行列の計算
			Matrix4x4 worldMatrix = Multiply(
				billboardMatrix,
				MakeAffineMatrix(particle.transform.scale, particle.transform.rotate, particle.transform.translate));
			// ビュー・プロジェクションを掛け合わせて最終行列を計算
			Matrix4x4 worldviewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
			// インスタンシングデータの設定
			if (group.second.instanceCount < kNumMaxInstance) {
				group.second.instancingDataPtr[group.second.instanceCount].WVP = worldviewProjectionMatrix;
				group.second.instancingDataPtr[group.second.instanceCount].World = worldMatrix;
				// カラーを設定し、アルファ値を減衰
				group.second.instancingDataPtr[group.second.instanceCount].color = particle.color;
				group.second.instancingDataPtr[group.second.instanceCount].color.w = 1.0f - (particle.currentTime / particle.lifeTime);
				if (group.second.instancingDataPtr[group.second.instanceCount].color.w < 0.0f) {
					group.second.instancingDataPtr[group.second.instanceCount].color.w = 0.0f;
				}
				// インスタンス数を増やす
				++group.second.instanceCount;
			}
			// 次のパーティクルへ
			++it;
		}
	}
}

/// <summary>
/// 描画処理
/// </summary>
void Particle::Draw() {
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList().Get();

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

	// 全てのパーティクルグループについて処理を行う
	for (auto& group : particleGroups) {
		if (group.second.instanceCount == 0) continue; // インスタンスが無い場合はスキップ

		Vector2 textureLeftTop = group.second.textureLeftTop;
		Vector2 textureSize = group.second.textureSize;

		// マテリアルCBufferの場所を設定
		commandList->SetGraphicsRootConstantBufferView(0, materialBuffer_->GetGPUVirtualAddress());

		// テクスチャのSRVのDescriptorTableを設定
		commandList->SetGraphicsRootDescriptorTable(2, particleCommon->GetSrvManager()->GetGPUDescriptorHandle(group.second.srvIndex));

		// インスタンシングデータのSRVのDescriptorTableを設定
		commandList->SetGraphicsRootDescriptorTable(1, particleCommon->GetSrvManager()->GetGPUDescriptorHandle(group.second.instancingSrvIndex));

		// Draw Call (インスタンシング描画)
		commandList->DrawInstanced(6, group.second.instanceCount, 0, 0);

		// インスタンスカウントをリセット
		group.second.instanceCount = 0;
	}
}

/// <summary>
/// パーティクルの生成
/// </summary>
/// <param name="name">パーティクル名</param>
/// <param name="position">生成位置</param>
/// <param name="count">生成数</param>
void Particle::Emit(const std::string name, const Vector3& position, uint32_t count) {
	if (particleGroups.find(name) == particleGroups.end()) {
		// パーティクルグループが存在しない場合はエラーを出力して終了
		assert("Specified particle group does not exist!");
	}

	// 指定されたパーティクルグループが存在する場合、そのグループにパーティクルを追加
	ParticleGroup& group = particleGroups[name];

	// すでにkNumMaxInstanceに達している場合、新しいパーティクルの追加をスキップする
	if (group.particleList.size() >= count) {
		return;
	}

	// 指定された数のパーティクルを生成して追加
	for (uint32_t i = 0; i < count; ++i) {
		group.particleList.push_back(CreateNewParticle(randomEngine_, position));
	}
}

/// <summary>
/// パーティクルグループの作成
/// </summary>
/// <param name="name">グループ名</param>
/// <param name="textureFilePath">テクスチャファイルパス</param>
void Particle::CreateParticleGroup(const std::string& name, const std::string& textureFilePath) {
	// 登録済みの名前かチェックして assert
	bool nameExists = false;
	for (auto it = particleGroups.begin(); it != particleGroups.end(); ++it) {
		if (it->second.materialFilePath == name) {
			nameExists = true;
			break;
		}
	}
	if (nameExists) {
		assert(false && "Particle group with this name already exists!");
	}

	// 新たなパーティクルグループを作成
	ParticleGroup newGroup;
	newGroup.materialFilePath = textureFilePath;

	// テクスチャのSRVインデックスを取得して設定
	TextureManager::GetInstance()->LoadTexture(textureFilePath);

	// テクスチャのSRVインデックスを取得して設定
	newGroup.srvIndex = TextureManager::GetInstance()->GetSrvIndex(textureFilePath);

	// テクスチャサイズを取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureFilePath);
	Vector2 textureSize = { static_cast<float>(metadata.width), static_cast<float>(metadata.height) };

	// サイズを設定（指定があればそれを使用、なければテクスチャサイズを使用）
	if (customTextureSize.x > 0.0f && customTextureSize.y > 0.0f) {
		newGroup.textureSize = customTextureSize;
	}
	else {
		newGroup.textureSize = textureSize;
	}

	// インスタンシング用リソースの生成
	newGroup.instancingResource = dxCommon_->CreateBufferResource(sizeof(ParticleForGPU) * kNumMaxInstance);

	newGroup.instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&newGroup.instancingDataPtr));
	for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
		newGroup.instancingDataPtr[index].WVP = MakeIdentity4x4();
		newGroup.instancingDataPtr[index].World = MakeIdentity4x4();
	}

	// インスタンシング用SRVを確保してSRVインデックスを記録
	newGroup.instancingSrvIndex = particleCommon->GetSrvManager()->Allocate() + 1;
	// 作成したSRVをインスタンシング用リソースに設定
	particleCommon->GetSrvManager()->CreateSRVForStructuredBuffer(newGroup.instancingSrvIndex, newGroup.instancingResource.Get(), kNumMaxInstance, sizeof(ParticleForGPU));

	// パーティクルグループをリストに追加
	particleGroups.emplace(name, newGroup);

	// マテリアルデータの初期化
	CreateMaterialData();
}

/// <summary>
/// 頂点データの作成
/// </summary>
void Particle::CreateVertexData() {
	modelData_.vertices.push_back(VertexData{ .position = {1.0f, 1.0f, 0.0f, 1.0f}, .texcoord = {0.0f, 0.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData_.vertices.push_back(VertexData{ .position = {-1.0f, 1.0f, 0.0f, 1.0f}, .texcoord = {1.0f, 0.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData_.vertices.push_back(VertexData{ .position = {1.0f, -1.0f, 0.0f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData_.vertices.push_back(VertexData{ .position = {1.0f, -1.0f, 0.0f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData_.vertices.push_back(VertexData{ .position = {-1.0f, 1.0f, 0.0f, 1.0f}, .texcoord = {1.0f, 0.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData_.vertices.push_back(VertexData{ .position = {-1.0f, -1.0f, 0.0f, 1.0f}, .texcoord = {1.0f, 1.0f}, .normal = {0.0f, 0.0f, 1.0f} });
}

/// <summary>
/// 頂点バッファビューの作成
/// </summary>
void Particle::CreateVertexBufferView() {
	// 頂点バッファの作成
	vertexBuffer_ = dxCommon_->CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size());

	// 頂点バッファビューの作成
	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点のサイズ
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

/// <summary>
/// マテリアルデータの作成
/// </summary>
void Particle::CreateMaterialData() {
	// マテリアル用のリソースを作成
	materialBuffer_ = dxCommon_->CreateBufferResource(sizeof(Material));

	// 書き込むためのアドレスを取得
	materialBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	// SpriteはLightingしないのfalseを設定する
	materialData_->enableLighting = false;
	materialData_->uvTransform = MakeIdentity4x4();
}

/// <summary>
/// 新しいパーティクルを生成
/// </summary>
/// <param name="randomEngine">ランダムエンジン</param>
/// <param name="position">生成位置</param>
/// <returns>新しいパーティクル情報</returns>
ParticleInfo Particle::CreateNewParticle(std::mt19937& randomEngine, const Vector3& position) {
	// カラーと寿命のランダム分布
	std::uniform_real_distribution<float> distColor(colorRange_.min, colorRange_.max);
	std::uniform_real_distribution<float> distTime(lifetimeRange_.min, lifetimeRange_.max);

	// 速度のランダム分布
	std::uniform_real_distribution<float> distSpeed(velocityRange_.min, velocityRange_.max);

	// 新たなパーティクルの生成
	ParticleInfo particle = {};

	particle.transform.scale = { 1.0f, 1.0f, 1.0f };
	particle.transform.rotate = { 0.0f, 0.0f, 0.0f };

	// 初期位置をエミッターの位置に設定
	particle.transform.translate = position;

	// ランダムな方向ベクトルの生成（球面上のランダムな点）
	std::uniform_real_distribution<float> distAngle(0.0f, 1.0f);
	float z = distAngle(randomEngine) * 2.0f - 1.0f; // z ∈ [-1, 1]
	float theta = distAngle(randomEngine) * 2.0f * std::numbers::pi_v<float>; // θ ∈ [0, 2π]
	float r = std::sqrt(1.0f - z * z);
	float x = r * std::cos(theta);
	float y = r * std::sin(theta);

	Vector3 direction = { x, y, z }; // 方向ベクトル

	// 速度を設定
	float speed = distSpeed(randomEngine);

	// 初期速度を設定
	particle.velocity = speed * direction;

	// カラーと寿命を設定
	particle.color = { distColor(randomEngine), distColor(randomEngine), distColor(randomEngine), 1.0f };
	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0.0f;

	return particle;
}

