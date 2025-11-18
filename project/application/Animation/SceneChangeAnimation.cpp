#include "SceneChangeAnimation.h"
#include "ImGuiManager.h"
#include "SceneChangeAnimation.h"
#include "TextureManager.h"
#include "DropWaveAnimation.h"

SceneChangeAnimation::SceneChangeAnimation(int screenWidth, int screenHeight, int blockSize, float duration, const std::string& blockTexturePath)
    : m_screenWidth(screenWidth), m_screenHeight(screenHeight), m_blockSize(blockSize), m_duration(duration), m_elapsed(0.0f), m_rng(std::random_device{}()), m_blockTexturePath(blockTexturePath),
      m_phase(Phase::Disappearing) {
	InitializeBlocks();
}

void SceneChangeAnimation::Initialize() {
	m_elapsed = 0.0f;
	m_phase = Phase::Disappearing;
	InitializeBlocks();
}

void SceneChangeAnimation::InitializeBlocks() {
	m_blocks.clear();

	int cols = (m_screenWidth + m_blockSize - 1) / m_blockSize;
	int rows = (m_screenHeight + m_blockSize - 1) / m_blockSize;
	float maxIndexSum = static_cast<float>((cols - 1) + (rows - 1));

	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			Block block;
			block.x = x * m_blockSize;
			block.y = y * m_blockSize;
			// フェーズによって初期アルファを切り替え
			if (m_phase == Phase::Disappearing) {
				block.alpha = 1.0f; // 覆いがある状態から消す
			} else {
				block.alpha = 0.0f; // 覆いがない状態から出す
			}
			float progress = (x + y) / maxIndexSum;
			float randomOffset = (static_cast<float>(m_rng() % 1000) / 1000.0f) * 0.15f;
			block.delay = (m_duration * 0.7f) * (progress + randomOffset * (1.0f - progress));
			block.fading = false;
			block.sprite = std::make_unique<Sprite>();
			TextureManager::GetInstance()->LoadTexture(m_blockTexturePath);
			block.sprite->Initialize(m_blockTexturePath);
			block.sprite->SetGetIsAdjustTextureSize(false);
			block.sprite->SetAnchorPoint({0.5f, 0.5f});
			block.sprite->SetPosition({static_cast<float>(block.x) + m_blockSize * 0.5f, static_cast<float>(block.y) + m_blockSize * 0.5f});
			block.sprite->SetSize({static_cast<float>(m_blockSize), static_cast<float>(m_blockSize)});
			
			// グラデーションカラー（赤→青）
			float r = 1.0f - progress;
			float g = 0.0f;
			float b = progress;
			block.sprite->SetColor({ r, g, b, block.alpha }); // ←ここもblock.alphaをセット
			block.sprite->Update();
			m_blocks.push_back(std::move(block));
		}
	}
}

void SceneChangeAnimation::Update(float deltaTime) {
	m_elapsed += deltaTime;

    if (m_phase == Phase::Appearing) {
        UpdateAppearing(deltaTime);
    } else if (m_phase == Phase::Disappearing) {
        UpdateDisappearing(deltaTime);
    }
}

void SceneChangeAnimation::UpdateAppearing(float deltaTime) {
    bool allAppeared = true;
    for (auto& block : m_blocks) {
        float t = 0.0f;
        if (m_elapsed > block.delay && block.alpha < 1.0f) {
            t = (m_elapsed - block.delay) / (m_duration * 0.3f);
            if (t > 1.0f) t = 1.0f;
            block.alpha = EaseInOut(t);
            if (block.alpha > 1.0f) block.alpha = 1.0f;
            auto color = block.sprite->GetColor();
            color.w = block.alpha;
            block.sprite->SetColor(color);

            float appearScale = m_appearScaleMin + (m_appearScaleMax - m_appearScaleMin) * EaseInOut(t);
            float appearRot = m_appearRotMax * (1.0f - EaseInOut(t));
            block.sprite->SetSize({m_blockSize * appearScale, m_blockSize * appearScale});
            block.sprite->SetRotation(appearRot);
        }
        if (block.alpha < 1.0f) allAppeared = false;
        block.sprite->Update();
    }
    if (allAppeared) {
        m_phase = Phase::Finished; // ←ここを追加
        m_elapsed = 0.0f;
    }
}

void SceneChangeAnimation::UpdateDisappearing(float deltaTime) {
    bool allDisappeared = true;
    for (auto& block : m_blocks) {
        float t = 0.0f;
        if (m_elapsed > block.delay && block.alpha > 0.0f) {
            t = (m_elapsed - block.delay) / (m_duration * 0.3f);
            if (t > 1.0f) t = 1.0f;
            block.alpha = 1.0f - EaseInOut(t);
            if (block.alpha < 0.0f) block.alpha = 0.0f;
            auto color = block.sprite->GetColor();
            color.w = block.alpha;
            block.sprite->SetColor(color);

            float disappearScale = m_disappearScaleMax - (m_disappearScaleMax - m_disappearScaleMin) * EaseInOut(t);
            float disappearRot = m_disappearRotMax * EaseInOut(t);
            block.sprite->SetSize({m_blockSize * disappearScale, m_blockSize * disappearScale});
            block.sprite->SetRotation(disappearRot);
        }
        if (block.alpha > 0.0f) allDisappeared = false;
        block.sprite->Update();
    }
    if (allDisappeared) {
        m_phase = Phase::Finished;
    }
}

// S字イージング関数
float SceneChangeAnimation::EaseInOut(float t) { return t < 0.5f ? 2.0f * t * t : 1.0f - powf(-2.0f * t + 2.0f, 2.0f) / 2.0f; }

void SceneChangeAnimation::Draw() const {
	for (const auto& block : m_blocks) {
		if (block.alpha > 0.0f) {
			block.sprite->Draw();
		}
	}
}

bool SceneChangeAnimation::IsFinished() const { return m_phase == Phase::Finished; }

// ImGuiコントロール
void SceneChangeAnimation::DrawImGui() {

#ifdef USE_IMGUI
	ImGui::Begin("SceneChangeAnimation");

	static int blockSize = m_blockSize;
	static float duration = m_duration;
	static char texturePath[128];
	

	strcpy_s(texturePath, sizeof(texturePath), m_blockTexturePath.c_str());

	bool needReinit = false;

	if (ImGui::InputInt("Block Size", &blockSize))
		needReinit = true;
	if (ImGui::InputFloat("Duration", &duration))
		needReinit = true;
	if (ImGui::InputText("Texture", texturePath, sizeof(texturePath)))
		needReinit = true;

	ImGui::Separator();
	ImGui::Text("Appear Animation");
	if (ImGui::SliderFloat("Appear Scale Min", &m_appearScaleMin, 0.1f, 1.0f)) {
	}
	if (ImGui::SliderFloat("Appear Scale Max", &m_appearScaleMax, 0.1f, 2.0f)) {
	}
	if (ImGui::SliderFloat("Appear Rot Max", &m_appearRotMax, 0.0f, 2.0f)) {
	}

	ImGui::Text("Disappear Animation");


	if (ImGui::SliderFloat("Disappear Scale Min", &m_disappearScaleMin, 0.1f, 1.0f)) {
	}
	if (ImGui::SliderFloat("Disappear Scale Max", &m_disappearScaleMax, 0.1f, 2.0f)) {
	}
	if (ImGui::SliderFloat("Disappear Rot Max", &m_disappearRotMax, 0.0f, 2.0f)) {
	}

	if (ImGui::Button("Reinitialize"))
		needReinit = true;

	if (needReinit) {
		m_blockSize = blockSize;
		m_duration = duration;
		m_blockTexturePath = texturePath;
		// ここで拡大率・回転量のパラメータもメンバ変数に保存する場合はセット
		Initialize();
	}

	ImGui::Text("Phase: %s", m_phase == Phase::Appearing ? "Appearing" : m_phase == Phase::Disappearing ? "Disappearing" : "Finished");
	ImGui::Text("IsFinished: %s", IsFinished() ? "Yes" : "No");

	ImGui::End();
#endif // USE_IMGUI
}

void SceneChangeAnimation::SetPhase(Phase phase) {
    m_phase = phase;
    m_elapsed = 0.0f;
    InitializeBlocks();
}
