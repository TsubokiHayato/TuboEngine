// Emit.h
#pragma once
#include "Particle.h"
#include "ParticleCommon.h"
#include <vector>

class ParticleEmitter {					
public:

    // コンストラクタ
	/// @param particle パーティクルのインスタンス
	/// @param name パーティクルグループ名
	/// @param transform エミッターの位置・回転・スケール
	/// @param count 発生させるパーティクルの数
	/// @param frequency 発生頻度
	/// @param repeat 繰り返し発生させるかどうかのフラグ
    ParticleEmitter(Particle* particle, const std::string& name, const Transform& transform, uint32_t count, float frequency, bool repeat = false);


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
    uint32_t count_;     // 発生させるパーティクルの数
    float frequency_;    // 発生頻度
    float elapsedTime_;  // 経過時間
    bool repeat_;        // 繰り返し発生させるかどうかのフラグ

};
