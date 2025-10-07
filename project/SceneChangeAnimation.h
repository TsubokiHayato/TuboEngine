#pragma once
#include <vector>
#include <random>
#include <memory>
#include "engine/graphic/2d/Sprite.h"

class SceneChangeAnimation {
public:
    SceneChangeAnimation(int screenWidth, int screenHeight, int blockSize, float duration, const std::string& blockTexturePath);

    void Initialize();
    void Update(float deltaTime);
    void Draw() const;
    bool IsFinished() const;
	static float EaseInOut(float t);
	void DrawImGui();

private:
    struct Block {
        int x, y;
        float alpha;
        float delay;
        bool fading;
        std::unique_ptr<Sprite> sprite;
    };
    
    enum class Phase { Appearing, Disappearing, Finished };

    int m_screenWidth;
    int m_screenHeight;
    int m_blockSize;
    float m_duration;
    float m_elapsed;
    std::vector<Block> m_blocks;
    std::mt19937 m_rng;
    std::string m_blockTexturePath;
	
	Phase m_phase;

	float m_appearScaleMin = 0.1f;
	float m_appearScaleMax = 1.0f;
	float m_appearRotMax = 2.0f;
	float m_disappearScaleMin = 0.1f;
	float m_disappearScaleMax = 1.0f;
	float m_disappearRotMax = 2.0f;

    void InitializeBlocks();
};
