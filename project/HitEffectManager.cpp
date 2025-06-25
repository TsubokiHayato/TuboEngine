#include "HitEffectManager.h"
#include <algorithm>

HitEffectManager* HitEffectManager::GetInstance() {
	static HitEffectManager instance;
	return &instance;
}

HitEffectManager::HitEffectManager() : particle(nullptr) {}

HitEffectManager::~HitEffectManager() {}

void HitEffectManager::Initialize(Particle* particle) { this->particle = particle; }

void HitEffectManager::PlayEffect(const Vector3& pos, HitEffectType type) {
	if (this->particle == nullptr) {
		return;
	}

	ParticleTransform transform;
	transform.scale = Vector3{1.0f, 1.0f, 1.0f};
	transform.rotate = Vector3{0.0f, 0.0f, 0.0f};
	transform.translate = pos;

	Vector3 velocity = Vector3{0.0f, 0.1f, 0.0f};
	Vector4 color = Vector4{1.0f, 0.5f, 0.2f, 1.0f};
	float lifeTime = 0.5f;
	float currentTime = 0.0f;
	int particleCount = 10;
	float frequency = 1.0f;
	bool isLoop = false;

	std::unique_ptr<ParticleEmitter> emitter(
		new ParticleEmitter(this->particle, 
			"Particle",
			transform, velocity, color, lifeTime, currentTime, particleCount, frequency, isLoop));

	this->emitters.push_back(std::move(emitter));
}

void HitEffectManager::Update() {
	for (size_t i = 0; i < this->emitters.size(); ++i) {
		this->emitters[i]->Update();
	}
	this->emitters.erase(std::remove_if(this->emitters.begin(), this->emitters.end(), [](const std::unique_ptr<ParticleEmitter>& e) { return e->IsDead(); }), this->emitters.end());
}

void HitEffectManager::Draw() {
	// ParticleEmitterのDrawは呼ばず、ParticleのDrawのみ呼ぶ
	if (this->particle != nullptr) {
		this->particle->Draw();
	}
}
