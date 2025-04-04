// Emit.cpp
#include "ParticleEmitter.h"
#include "Camera.h"
#include <cmath>
#include"MT_Matrix.h"
#include "TextureManager.h"
#include <random>
#define _USE_MATH_DEFINES
#include <math.h>


ParticleEmitter::ParticleEmitter(Particle* particle, const std::string& name, const Transform& transform, uint32_t count, float frequency, bool repeat)
    : particle_(particle), name_(name), transform_(transform), count_(count), frequency_(frequency), elapsedTime_(frequency), repeat_(repeat)
{
    Emit(); // 初期化時に即時発生
}

void ParticleEmitter::Update() {
	if (!repeat_) return; // 繰り返し発生させない場合は何もしない

    elapsedTime_ += 1.0f / 60.0f; // フレーム単位の経過時間を加算

	// 経過時間が発生頻度を超えたらパーティクルを発生させる
    if (elapsedTime_ >= frequency_)
    {
		// パーティクルの発生
        Emit();
        // 周期的に実行するためリセット
        elapsedTime_ -= frequency_;
    }
}

void ParticleEmitter::Draw() {

}

void ParticleEmitter::Emit() {
	// パーティクルの発生
    particle_->Emit(name_, transform_.translate, count_);
}

void ParticleEmitter::SetRepeat(bool repeat) {
	// 繰り返し発生させるかどうかのフラグを設定
    repeat_ = repeat;
}
