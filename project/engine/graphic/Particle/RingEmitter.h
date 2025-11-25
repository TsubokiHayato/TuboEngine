#pragma once
#include "IParticleEmitter.h"
#include <numbers>

class RingEmitter : public IParticleEmitter {
protected:
    void BuildGeometry(std::vector<VertexData>& out) override {
        const uint32_t div = 64;
        const float outerR = 1.0f;
        const float innerR = 0.2f;
        float step = 2.0f * std::numbers::pi_v<float> / float(div);
        for (uint32_t i=0;i<div;i++){
            float a0 = i * step;
            float a1 = (i+1) * step;
            float s0 = std::sin(a0), c0 = std::cos(a0);
            float s1 = std::sin(a1), c1 = std::cos(a1);
            float u0 = float(i)/float(div);
            float u1 = float(i+1)/float(div);
            out.push_back({{-s0*innerR, c0*innerR,0,1},{u0,1},{0,0,1}});
            out.push_back({{-s1*innerR, c1*innerR,0,1},{u1,1},{0,0,1}});
            out.push_back({{-s1*outerR, c1*outerR,0,1},{u1,0},{0,0,1}});
            out.push_back({{-s0*outerR, c0*outerR,0,1},{u0,0},{0,0,1}});
            out.push_back({{-s0*innerR, c0*innerR,0,1},{u0,1},{0,0,1}});
            out.push_back({{-s1*outerR, c1*outerR,0,1},{u1,0},{0,0,1}});
        }
    }
    ParticleInfo GenerateParticle() override {
        std::uniform_real_distribution<float> life(preset_.lifeMin, preset_.lifeMax);
        std::uniform_real_distribution<float> sx(preset_.scaleMin.x, preset_.scaleMax.x);
        ParticleInfo p{};
        p.transform.scale = { sx(rng_), sx(rng_), 1.0f };
        p.transform.rotate = {0,0,0};
        p.transform.translate = Vector3{
            std::uniform_real_distribution<float>(preset_.posMin.x,preset_.posMax.x)(rng_),
            std::uniform_real_distribution<float>(preset_.posMin.y,preset_.posMax.y)(rng_),
            std::uniform_real_distribution<float>(preset_.posMin.z,preset_.posMax.z)(rng_)
        };
        p.velocity = {
            std::uniform_real_distribution<float>(preset_.velMin.x,preset_.velMax.x)(rng_),
            std::uniform_real_distribution<float>(preset_.velMin.y,preset_.velMax.y)(rng_),
            std::uniform_real_distribution<float>(preset_.velMin.z,preset_.velMax.z)(rng_)
        };
        p.color = {
            std::uniform_real_distribution<float>(preset_.colMin.x,preset_.colMax.x)(rng_),
            std::uniform_real_distribution<float>(preset_.colMin.y,preset_.colMax.y)(rng_),
            std::uniform_real_distribution<float>(preset_.colMin.z,preset_.colMax.z)(rng_),
            std::uniform_real_distribution<float>(preset_.colMin.w,preset_.colMax.w)(rng_)
        };
        p.lifeTime = life(rng_);
        p.currentTime = 0.0f;
        return p;
    }
};
