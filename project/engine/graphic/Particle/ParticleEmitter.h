// Emit.h
#pragma once
#include "Particle.h"
#include "ParticleCommon.h"
#include <vector>

class ParticleEmitter {					
public:

    // コンストラクタ
	/// @brief パーティクルエミッターのコンストラクタ
	/// @param particle パーティクルのインスタンス
	/// @param name パーティクルグループ名
	/// @param transform エミッターの位置・回転・スケール
	/// @param velocity 速度
	/// @param color カラー
	/// @param lifeTime 寿命
	/// @param currentTime 経過時間
	/// @param count 発生させるパーティクルの数
	/// @param frequency 発生頻度
	/// @param repeat 繰り返し発生させるかどうかのフラグ
	ParticleEmitter(Particle* particle, const std::string& name, 
		const Transform& transform, Vector3 velocity, Vector4 color, float lifeTime, float currentTime,
		uint32_t count, float frequency, bool repeat = false);


	//更新
    void Update();

	//描画
    void Draw();

	//エミッション
    void Emit();

	//繰り返し設定
    void SetRepeat(bool repeat);


public:
	///-----------------------------------------------------------------
	/// Getters & Setters
	/// 
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	Vector3 GetScale() { return transform_.scale; }
	void SetPosition(const Vector3& position) { transform_.translate = position; }
	Vector3 GetPosition() { return transform_.translate; }
	void SetRotation(const Vector3& rotation) { transform_.rotate = rotation; }
	Vector3 GetRotation() { return transform_.rotate; }
	void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }
	Vector3 GetVelocity() { return velocity_; }
	void SetColor(const Vector4& color) { color_ = color; }
	Vector4 GetColor() { return color_; }
	void SetLifeTime(float lifeTime) { lifeTime_ = lifeTime; }
	float GetLifeTime() { return lifeTime_; }
	void SetCurrentTime(float currentTime) { currentTime_ = currentTime; }
	float GetCurrentTime() { return currentTime_; }
	void SetCount(uint32_t count) { count_ = count; }


private:
    Particle* particle_; // Particleのインスタンスを保持
    std::string name_;   // パーティクルグループ名
    Transform transform_;// エミッターの位置・回転・スケール
	Vector3 velocity_; // 速度
	Vector4 color_;    // カラー
	float lifeTime_;   // 寿命
	float currentTime_; // 経過時間
    uint32_t count_;     // 発生させるパーティクルの数
    float frequency_;    // 発生頻度
    float elapsedTime_;  // 経過時間
    bool repeat_;        // 繰り返し発生させるかどうかのフラグ

};
