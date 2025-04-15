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
		const Transform& transform, Vector2 velocity, Vector4 color, float lifeTime, float currentTime,
		uint32_t count, float frequency, bool repeat = false);


	//更新
    void Update();

	//描画
    void Draw();

	//エミッション
    void Emit();

	//繰り返し設定
    void SetRepeat(bool repeat);

private:
    Particle* particle_; // Particleのインスタンスを保持
    std::string name_;   // パーティクルグループ名
    Transform transform_;// エミッターの位置・回転・スケール
	Vector4 velocity_; // 速度
	Vector4 color_;    // カラー
	float lifeTime_;   // 寿命
	float currentTime_; // 経過時間
    uint32_t count_;     // 発生させるパーティクルの数
    float frequency_;    // 発生頻度
    float elapsedTime_;  // 経過時間
    bool repeat_;        // 繰り返し発生させるかどうかのフラグ

};
