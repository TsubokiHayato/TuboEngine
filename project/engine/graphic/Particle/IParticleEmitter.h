#pragma once
#include <string>
#include <list>
#include <memory>
#include <random>
#include "Transform.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "ParticleCommon.h"
#include "TextureManager.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "Material.h"
#include "VertexData.h"

struct ParticleInfo {
    Transform transform;
    Vector3 velocity;
    Vector4 color;
    float lifeTime;
    float currentTime;
};

struct ParticleForGPU {
    Matrix4x4 WVP;
    Matrix4x4 World;
    Vector4 color;
};

struct ParticlePreset {
    // 基本
    std::string name;
    std::string texture;
    uint32_t maxInstances = 128;
    bool billboard = true;
    // レンジ
    Vector3 posMin{0,0,0}, posMax{0,0,0};
    Vector3 velMin{0,0,0}, velMax{0,0,0};
    Vector3 scaleMin{1,1,1}, scaleMax{1,1,1};
    Vector4 colMin{1,1,1,1}, colMax{1,1,1,1};
    float lifeMin = 0.5f, lifeMax = 1.5f;
    // Emit
    bool autoEmit = false;
    float emitRate = 30.0f; // 個/秒
    uint32_t burstCount = 10;
};

class IParticleEmitter {
public:
    virtual ~IParticleEmitter() = default;
    virtual void Initialize(const ParticlePreset& preset);
    virtual void Update(float dt, const Camera* camera);
    virtual void Draw(ID3D12GraphicsCommandList* cmd);
    virtual void Emit(uint32_t count); // 手動バースト
    virtual ParticlePreset& GetPreset() { return preset_; }
    virtual const ParticlePreset& GetPreset() const { return preset_; }
    virtual const std::string& GetName() const { return preset_.name; }
    virtual void DrawImGui(); // 個別パラメータ編集

protected:
    // 派生で粒子1個生成
    virtual ParticleInfo GenerateParticle() = 0;
    // 派生で頂点形状生成
    virtual void BuildGeometry(std::vector<VertexData>& outVertices) = 0;

    void EnsureBuffers();
    void PushParticle(const ParticleInfo& p);
    void UpdateParticles(float dt, const Matrix4x4& viewProj, const Matrix4x4& billboard);
    void ResetInstances();

protected:
    ParticlePreset preset_{};
    std::list<ParticleInfo> particles_;
    std::mt19937 rng_{ std::random_device{}() };

    // GPU
    Microsoft::WRL::ComPtr<ID3D12Resource> vb_;
    Microsoft::WRL::ComPtr<ID3D12Resource> material_;
    Material* materialPtr_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> instancing_;
    ParticleForGPU* instancingPtr_ = nullptr;
    uint32_t instanceCount_ = 0;
    int textureSrvIndex_ = -1;
    int instancingSrvIndex_ = -1;

    std::vector<VertexData> vertices_;
};