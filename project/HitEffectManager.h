//#pragma once
//#include "Object3dCommon.h"
//#include "Particle.h"
//#include "ParticleEmitter.h"
//#include "Vector3.h"
//#include <memory>
//#include <vector>
//
//enum class HitEffectType {
//	Default,
//	// 必要に応じて追加
//};
//
//struct ParticleTransform
//{
//	Vector3 scale;
//	Vector3 rotate;
//	Vector3 translate;
//};
//
//class HitEffectManager {
//public:
//	static HitEffectManager* GetInstance();
//
//	// 初期化（Particleをセット）
//	void Initialize(Particle* particle);
//
//	// エフェクト発生
//	void PlayEffect(const Vector3& pos, HitEffectType type = HitEffectType::Default);
//
//	// 更新処理
//	void Update();
//
//	// 描画処理（ParticleのDrawを呼ぶ）
//	void Draw();
//
//private:
//	HitEffectManager();
//	~HitEffectManager();
//	HitEffectManager(const HitEffectManager&) = delete;
//	HitEffectManager& operator=(const HitEffectManager&) = delete;
//
//	Particle* particle;
//	std::vector<std::unique_ptr<ParticleEmitter>> emitters;
//};
