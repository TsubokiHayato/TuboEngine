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
        std::uniform_real_distribution<float> rotZ(0.0f, 3.14159f);
        std::uniform_real_distribution<float> scaleY(preset_.scaleMin.y, preset_.scaleMax.y);
        p.transform.scale = {0.025f, scaleY(rng_), 1.0f};
        p.transform.rotate = {0,0,rotZ(rng_)};
        p.transform.translate = {
            std::uniform_real_distribution<float>(preset_.posMin.x,preset_.posMax.x)(rng_),
            std::uniform_real_distribution<float>(preset_.posMin.y,preset_.posMax.y)(rng_),
            std::uniform_real_distribution<float>(preset_.posMin.z,preset_.posMax.z)(rng_)
        };
        p.velocity = {0,0,0};
        p.color = {1,1,1,1};
        p.lifeTime = life(rng_);
        p.currentTime = 0.0f;
        return p;
    }
};