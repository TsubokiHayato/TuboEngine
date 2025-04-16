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
	} else {
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
		/*	particle.transform.scale.x = textureSize.x * scaleMultiplier;
			particle.transform.scale.y = textureSize.y * scaleMultiplier;*/
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
void Particle::Emit(const std::string name, const Transform& transform,Vector3 velocity, Vector4 color,float lifeTime, float currentTime, uint32_t count) {
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
		group.particleList.push_back(CreateNewParticle(randomEngine_, transform, velocity, color, lifeTime, currentTime));
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
	} else {
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
/// <param name="velocity">速度</param>
/// <param name="color">カラー</param>
/// <param name="lifeTime">寿命</param>
/// <param name="currentTime">経過時間</param>
/// <returns>新しいパーティクル情報</returns>
/// 
ParticleInfo Particle::CreateNewParticle(std::mt19937& randomEngine, const Transform& transform, Vector3 velocity, Vector4 color, float lifeTime, float currentTime) {
	// 新たなパーティクルの生成
	ParticleInfo particle = {};



	// 回転範囲の設定
	std::uniform_real_distribution<float> distRotateX(std::min(rotateRange_.x.min, rotateRange_.x.max), std::max(rotateRange_.x.min, rotateRange_.x.max));
	std::uniform_real_distribution<float> distRotateY(std::min(rotateRange_.y.min, rotateRange_.y.max), std::max(rotateRange_.y.min, rotateRange_.y.max));
	std::uniform_real_distribution<float> distRotateZ(std::min(rotateRange_.z.min, rotateRange_.z.max), std::max(rotateRange_.z.min, rotateRange_.z.max));
	// 拡大縮小範囲の設定
	std::uniform_real_distribution<float> distScaleX(std::min(scaleRange_.x.min, scaleRange_.x.max), std::max(scaleRange_.x.min, scaleRange_.x.max));
	std::uniform_real_distribution<float> distScaleY(std::min(scaleRange_.y.min, scaleRange_.y.max), std::max(scaleRange_.y.min, scaleRange_.y.max));
	std::uniform_real_distribution<float> distScaleZ(std::min(scaleRange_.z.min, scaleRange_.z.max), std::max(scaleRange_.z.min, scaleRange_.z.max));
	// 平行移動範囲の設定
	std::uniform_real_distribution<float> distTranslateX(std::min(translateRange_.x.min, translateRange_.x.max), std::max(translateRange_.x.min, translateRange_.x.max));
	std::uniform_real_distribution<float> distTranslateY(std::min(translateRange_.y.min, translateRange_.y.max), std::max(translateRange_.y.min, translateRange_.y.max));
	std::uniform_real_distribution<float> distTranslateZ(std::min(translateRange_.z.min, translateRange_.z.max), std::max(translateRange_.z.min, translateRange_.z.max));
	// 速度範囲の設定
	std::uniform_real_distribution<float> distVelocityX(std::min(velocityRange_.x.min, velocityRange_.x.max), std::max(velocityRange_.x.min, velocityRange_.x.max));
	std::uniform_real_distribution<float> distVelocityY(std::min(velocityRange_.y.min, velocityRange_.y.max), std::max(velocityRange_.y.min, velocityRange_.y.max));
	std::uniform_real_distribution<float> distVelocityZ(std::min(velocityRange_.z.min, velocityRange_.z.max), std::max(velocityRange_.z.min, velocityRange_.z.max));
	// 色範囲の設定
	std::uniform_real_distribution<float> distColorR(std::min(colorRange_.x.min, colorRange_.x.max), std::max(colorRange_.x.min, colorRange_.x.max));
	std::uniform_real_distribution<float> distColorG(std::min(colorRange_.y.min, colorRange_.y.max), std::max(colorRange_.y.min, colorRange_.y.max));
	std::uniform_real_distribution<float> distColorB(std::min(colorRange_.z.min, colorRange_.z.max), std::max(colorRange_.z.min, colorRange_.z.max));
	std::uniform_real_distribution<float> distColorA(std::min(colorRange_.w.min, colorRange_.w.max), std::max(colorRange_.w.min, colorRange_.w.max));
	// 寿命範囲の設定
	std::uniform_real_distribution<float> distLifeTime(std::min(lifetimeRange_.min, lifetimeRange_.max), std::max(lifetimeRange_.min, lifetimeRange_.max));
	// 経過時間範囲の設定
	std::uniform_real_distribution<float> distCurrentTime(std::min(currentTimeRange_.min, currentTimeRange_.max), std::max(currentTimeRange_.min, currentTimeRange_.max));

	particle.transform.translate = {};
	particle.transform.rotate = { distRotateX(randomEngine),distRotateY(randomEngine),distRotateZ(randomEngine) };
	particle.transform.scale = { 0.025f, distScaleY(randomEngine), 1.0f };

	// 拡大縮小、回転、平行移動の設定
	// particle.transform = transform;
	// 速度の設定
	particle.velocity = velocity;
	// 色の設定
	particle.color = color;
	// 寿命の設定
	particle.lifeTime = lifeTime;
	// 経過時間の設定
	particle.currentTime = currentTime;

	return particle;
}

