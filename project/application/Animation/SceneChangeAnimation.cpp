#include "SceneChangeAnimation.h"

#include "ImGuiManager.h"
#include "TextureManager.h"

#include <algorithm>
#include <cmath>

SceneChangeAnimation::SceneChangeAnimation(int screenWidth, int screenHeight, int blockSize, float duration,
	const std::string& blockTexturePath)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
    , m_blockSize(blockSize)
    , m_duration(duration)
    , m_elapsed(0.0f)
    , m_rng(std::random_device{}())
    , m_blockTexturePath(blockTexturePath)
    , m_phase(Phase::Disappearing) {
	InitializeBlocks();
}

void SceneChangeAnimation::Initialize() {
	m_elapsed = 0.0f;
	m_phase = Phase::Disappearing;
	InitializeBlocks();
}

void SceneChangeAnimation::InitializeBlocks() {
	m_blocks.clear();

	// ブロック数（画面をはみ出す分は切り上げ）
	const int cols = (m_screenWidth + m_blockSize - 1) / m_blockSize;
	const int rows = (m_screenHeight + m_blockSize - 1) / m_blockSize;
	const float maxIndexSum = static_cast<float>((cols - 1) + (rows - 1));

	// テクスチャは全ブロックで共通なので、生成前に一度だけロードする。
	TextureManager::GetInstance()->LoadTexture(m_blockTexturePath);

	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			BlockSprite block;
			block.x = x * m_blockSize;
			block.y = y * m_blockSize;

			// フェーズによって初期アルファを切り替える
			// - Disappearing: 覆いがある状態(1.0)から消す
			// - Appearing   : 覆いがない状態(0.0)から出す
			block.alpha = (m_phase == Phase::Disappearing) ? 1.0f : 0.0f;

			// 左上→右下に流れるように開始する（x+y で進行度を作る）
			const float progress = (maxIndexSum > 0.0f) ? (static_cast<float>(x + y) / maxIndexSum) : 0.0f;

			// 単調に見えないよう小さなランダム遅延を足す
			const float randomOffset = (static_cast<float>(m_rng() % 1000) / 1000.0f) * 0.15f;
			block.delay = (m_duration * 0.7f) * (progress + randomOffset * (1.0f - progress));

			block.sprite = std::make_unique<Sprite>();
			block.sprite->Initialize(m_blockTexturePath);
			block.sprite->SetGetIsAdjustTextureSize(false);
			block.sprite->SetAnchorPoint({0.5f, 0.5f});
			block.sprite->SetPosition({static_cast<float>(block.x) + m_blockSize * 0.5f,
				static_cast<float>(block.y) + m_blockSize * 0.5f});
			block.sprite->SetSize({static_cast<float>(m_blockSize), static_cast<float>(m_blockSize)});

			// 視認性用：簡易グラデーション（赤→青）
			const float r = 1.0f - progress;
			const float g = 0.0f;
			const float b = progress;
			block.sprite->SetColor({r, g, b, block.alpha});

			block.sprite->Update();
			m_blocks.push_back(std::move(block));
		}
	}
}

void SceneChangeAnimation::Update(float deltaTime) {
	m_elapsed += deltaTime;

	switch (m_phase) {
	case Phase::Appearing:
		UpdateAppearing(deltaTime);
		break;
	case Phase::Disappearing:
		UpdateDisappearing(deltaTime);
		break;
	case Phase::Finished:
	default:
		break;
	}
}

void SceneChangeAnimation::UpdateAppearing(float /*deltaTime*/) {
	bool allAppeared = true;

	for (auto& block : m_blocks) {
		if (m_elapsed > block.delay && block.alpha < 1.0f) {
			float t = (m_elapsed - block.delay) / (m_duration * 0.3f);
			t = std::clamp(t, 0.0f, 1.0f);

			block.alpha = std::clamp(EaseInOut(t), 0.0f, 1.0f);

			auto color = block.sprite->GetColor();
			color.w = block.alpha;
			block.sprite->SetColor(color);

			// 出現時：小さく回転しつつ等倍へ
			const float eased = EaseInOut(t);
			const float appearScale = m_appearScaleMin + (m_appearScaleMax - m_appearScaleMin) * eased;
			const float appearRot = m_appearRotMax * (1.0f - eased);
			block.sprite->SetSize({m_blockSize * appearScale, m_blockSize * appearScale});
			block.sprite->SetRotation(appearRot);
		}

		if (block.alpha < 1.0f) {
			allAppeared = false;
		}

		block.sprite->Update();
	}

	if (allAppeared) {
		m_phase = Phase::Finished;
		m_elapsed = 0.0f;
	}
}

void SceneChangeAnimation::UpdateDisappearing(float /*deltaTime*/) {
	bool allDisappeared = true;

	for (auto& block : m_blocks) {
		if (m_elapsed > block.delay && block.alpha > 0.0f) {
			float t = (m_elapsed - block.delay) / (m_duration * 0.3f);
			t = std::clamp(t, 0.0f, 1.0f);

			block.alpha = std::clamp(1.0f - EaseInOut(t), 0.0f, 1.0f);

			auto color = block.sprite->GetColor();
			color.w = block.alpha;
			block.sprite->SetColor(color);

			// 消失時：等倍から縮小しつつ回転
			const float eased = EaseInOut(t);
			const float disappearScale = m_disappearScaleMax - (m_disappearScaleMax - m_disappearScaleMin) * eased;
			const float disappearRot = m_disappearRotMax * eased;
			block.sprite->SetSize({m_blockSize * disappearScale, m_blockSize * disappearScale});
			block.sprite->SetRotation(disappearRot);
		}

		if (block.alpha > 0.0f) {
			allDisappeared = false;
		}

		block.sprite->Update();
	}

	if (allDisappeared) {
		m_phase = Phase::Finished;
	}
}

float SceneChangeAnimation::EaseInOut(float t) {
	// S字イージング（0..1 → 0..1）
	return (t < 0.5f) ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}

void SceneChangeAnimation::Draw() const {
	for (const auto& block : m_blocks) {
		if (block.alpha > 0.0f) {
			block.sprite->Draw();
		}
	}
}

bool SceneChangeAnimation::IsFinished() const { return m_phase == Phase::Finished; }

void SceneChangeAnimation::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("SceneChangeAnimation");

	static int blockSize = m_blockSize;
	static float duration = m_duration;
	static char texturePath[128];
	strcpy_s(texturePath, sizeof(texturePath), m_blockTexturePath.c_str());

	bool needReinit = false;

	needReinit |= ImGui::InputInt("Block Size", &blockSize);
	needReinit |= ImGui::InputFloat("Duration", &duration);
	needReinit |= ImGui::InputText("Texture", texturePath, sizeof(texturePath));

	ImGui::Separator();
	ImGui::Text("Appear Animation");
	ImGui::SliderFloat("Appear Scale Min", &m_appearScaleMin, 0.1f, 1.0f);
	ImGui::SliderFloat("Appear Scale Max", &m_appearScaleMax, 0.1f, 2.0f);
	ImGui::SliderFloat("Appear Rot Max", &m_appearRotMax, 0.0f, 2.0f);

	ImGui::Text("Disappear Animation");
	ImGui::SliderFloat("Disappear Scale Min", &m_disappearScaleMin, 0.1f, 1.0f);
	ImGui::SliderFloat("Disappear Scale Max", &m_disappearScaleMax, 0.1f, 2.0f);
	ImGui::SliderFloat("Disappear Rot Max", &m_disappearRotMax, 0.0f, 2.0f);

	needReinit |= ImGui::Button("Reinitialize");

	if (needReinit) {
		m_blockSize = blockSize;
		m_duration = duration;
		m_blockTexturePath = texturePath;
		Initialize();
	}

	ImGui::Text("Phase: %s",
		m_phase == Phase::Appearing ? "Appearing" : m_phase == Phase::Disappearing ? "Disappearing" : "Finished");
	ImGui::Text("IsFinished: %s", IsFinished() ? "Yes" : "No");

	ImGui::End();
#endif // USE_IMGUI
}

void SceneChangeAnimation::SetPhase(Phase phase) {
	m_phase = phase;
	m_elapsed = 0.0f;
	InitializeBlocks();
}
