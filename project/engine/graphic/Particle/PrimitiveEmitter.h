#pragma once
#include "IParticleEmitter.h"

class PrimitiveEmitter : public IParticleEmitter {
protected:
    void BuildGeometry(std::vector<VertexData>& out) override {
        // シンプルな板
        out.push_back({{ 1, 1,0,1},{0,0},{0,0,1}});
        out.push_back({{-1, 1,0,1},{1,0},{0,0,1}});
        out.push_back({{ 1,-1,0,1},{0,1},{0,0,1}});
        out.push_back({{ 1,-1,0,1},{0,1},{0,0,1}});
        out.push_back({{-1, 1,0,1},{1,0},{0,0,1}});
        out.push_back({{-1,-1,0,1},{1,1},{0,0,1}});
    }
    ParticleInfo GenerateParticle() override {
        ParticleInfo p{};
        std::uniform_real_distribution<float> life(preset_.lifeMin, preset_.lifeMax);
        std::uniform_real_distribution<float> rotZ(preset_.initialRotRangeZ.x, preset_.initialRotRangeZ.y);
        std::uniform_real_distribution<float> rotSpd(preset_.rotSpeedRangeZ.x, preset_.rotSpeedRangeZ.y);
        std::uniform_real_distribution<float> scaleY(preset_.scaleMin.y, preset_.scaleMax.y);

        p.transform.scale = {0.025f, scaleY(rng_), 1.0f};
        p.transform.rotate = {0,0,rotZ(rng_)};
        p.transform.translate = {
            std::uniform_real_distribution<float>(preset_.posMin.x,preset_.posMax.x)(rng_),
            std::uniform_real_distribution<float>(preset_.posMin.y,preset_.posMax.y)(rng_),
            std::uniform_real_distribution<float>(preset_.posMin.z,preset_.posMax.z)(rng_)
        };
        p.velocity = {
            std::uniform_real_distribution<float>(preset_.velMin.x,preset_.velMax.x)(rng_),
            std::uniform_real_distribution<float>(preset_.velMin.y,preset_.velMax.y)(rng_),
            std::uniform_real_distribution<float>(preset_.velMin.z,preset_.velMax.z)(rng_)
        };
        p.color = preset_.colorStart;
        p.lifeTime = life(rng_);
        p.currentTime = 0.0f;
        // 拡張: 回転速度を color.w に埋めるか別フィールド追加推奨
        // 簡易的に velocity.w を使いたいなら構造体拡張
        return p;
    }
};