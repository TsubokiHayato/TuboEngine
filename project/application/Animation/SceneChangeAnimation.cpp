#include "SceneChangeAnimation.h"

#include "ImGuiManager.h"
#include "TextureManager.h"

#include <algorithm>
#include <cmath>

namespace {
	// ブロックの更新に使う係数。
	// duration 全体に対し、個々のブロックがフェードする時間割合。
	constexpr float kFadeDurationRatio = 0.3f;

	// ブロックの遅延計算に使う係数。
	// duration 全体に対して、遅延を割り当てる時間割合（残りはフェードに使う想定）。
	constexpr float kDelayDurationRatio = 0.7f;

	// 遅延に足すランダムばらつき量（最大）。
	constexpr float kDelayRandomJitterMax = 0.15f;
}

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
	// 初回生成
	InitializeBlocks();
}

void SceneChangeAnimation::Initialize() {
	// 時間・フェーズを初期状態に戻す
	m_elapsed = 0.0f;
	m_phase = Phase::Disappearing;

	// ブロックを作り直す
	InitializeBlocks();
}

void SceneChangeAnimation::InitializeBlocks() {
	// --- 生成済みブロックを破棄 ---
	m_blocks.clear();

	// --- 画面をブロックで敷き詰めるための列・行数を求める ---
	// 端が割り切れない場合は、最後の列/行を追加するため切り上げ。
	const int cols = (m_screenWidth + m_blockSize - 1) / m_blockSize;
	const int rows = (m_screenHeight + m_blockSize - 1) / m_blockSize;

	// x+y を 0..1 に正規化するための最大値。
	// cols/rows が 1 のケースもあり得るため、後段で 0 除算を避ける。
	const float maxIndexSum = static_cast<float>((cols - 1) + (rows - 1));

	// --- テクスチャ準備 ---
	// 全ブロックで共通なので、ループの外で一度だけロード。
	TuboEngine::TextureManager::GetInstance()->LoadTexture(m_blockTexturePath);

	// --- ブロック生成 ---
	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			BlockSprite block;

			// 左上原点でのブロック左上座標（px）
			block.x = x * m_blockSize;
			block.y = y * m_blockSize;

			// フェーズによって初期アルファを切り替える。
			// Disappearing: 画面を覆った状態(1.0)から消す
			// Appearing   : 何も無い状態(0.0)から出す
			block.alpha = (m_phase == Phase::Disappearing) ? 1.0f : 0.0f;

			// --- 遅延計算 ---
			// x+y で左上→右下に向かうほど進行度が増える。
			// 0..1 に正規化して使う。
			const float progress = (maxIndexSum > 0.0f) ? (static_cast<float>(x + y) / maxIndexSum) : 0.0f;

			// 単調にならないようランダム要素を追加。
			// progress が大きいほど影響を小さくして、終端側の破綻を抑える。
			const float randomOffset = (static_cast<float>(m_rng() % 1000) / 1000.0f) * kDelayRandomJitterMax;
			block.delay = (m_duration * kDelayDurationRatio) * (progress + randomOffset * (1.0f - progress));

			// --- スプライト生成 ---
			block.sprite = std::make_unique<TuboEngine::Sprite>();
			block.sprite->Initialize(m_blockTexturePath);

			// このスプライトは「ブロックサイズ」で描画するため、テクスチャサイズ自動調整は無効。
			block.sprite->SetGetIsAdjustTextureSize(false);

			// ブロック中心を基準に拡縮・回転させたいので中心アンカー。
			block.sprite->SetAnchorPoint({0.5f, 0.5f});

			// SetPosition はアンカー基準の座標なので、ブロック中心へ設定。
			block.sprite->SetPosition({static_cast<float>(block.x) + m_blockSize * 0.5f,
				static_cast<float>(block.y) + m_blockSize * 0.5f});

			// 初期サイズは等倍（blockSize x blockSize）
			block.sprite->SetSize({static_cast<float>(m_blockSize), static_cast<float>(m_blockSize)});

			// --- 色設定 ---
			// 視認性用：簡易グラデーション（赤→青）。
			// 本番で単色にしたい場合はここを固定値にする。
			const float r = 1.0f - progress;
			const float g = 0.0f;
			const float b = progress;
			block.sprite->SetColor({r, g, b, block.alpha});

			// 生成直後に内部バッファへ反映
			block.sprite->Update();

			m_blocks.push_back(std::move(block));
		}
	}
}

void SceneChangeAnimation::Update(float deltaTime) {
	// --- 経過時間の更新 ---
	m_elapsed += deltaTime;

	// --- フェーズに応じた更新 ---
	switch (m_phase) {
	case Phase::Appearing:
		UpdateAppearing(deltaTime);
		break;
	case Phase::Disappearing:
		UpdateDisappearing(deltaTime);
		break;
	case Phase::Finished:
	default:
		// Finished は何もしない
		break;
	}
}

void SceneChangeAnimation::UpdateAppearing(float /*deltaTime*/) {
	// すべてのブロックが alpha==1 に到達したら終了
	bool allAppeared = true;

	for (auto& block : m_blocks) {
		// --- ブロックごとの時間（開始遅延を考慮）---
		if (m_elapsed > block.delay && block.alpha < 1.0f) {
			// 0..1 に正規化した進行度
			float t = (m_elapsed - block.delay) / (m_duration * kFadeDurationRatio);
			t = std::clamp(t, 0.0f, 1.0f);

			// --- アルファ更新 ---
			block.alpha = std::clamp(EaseInOut(t), 0.0f, 1.0f);

			auto color = block.sprite->GetColor();
			color.w = block.alpha;
			block.sprite->SetColor(color);

			// --- 見た目（拡縮・回転）更新 ---
			// 出現時：小さく回転しつつ等倍へ
			const float eased = EaseInOut(t);
			const float appearScale = m_appearScaleMin + (m_appearScaleMax - m_appearScaleMin) * eased;
			const float appearRot = m_appearRotMax * (1.0f - eased);
			block.sprite->SetSize({m_blockSize * appearScale, m_blockSize * appearScale});
			block.sprite->SetRotation(appearRot);
		}

		// --- 完了判定 ---
		if (block.alpha < 1.0f) {
			allAppeared = false;
		}

		// 変更を描画に反映
		block.sprite->Update();
	}

	// --- フェーズ完了 ---
	if (allAppeared) {
		m_phase = Phase::Finished;
		m_elapsed = 0.0f;
	}
}

void SceneChangeAnimation::UpdateDisappearing(float /*deltaTime*/) {
	// すべてのブロックが alpha==0 に到達したら終了
	bool allDisappeared = true;

	for (auto& block : m_blocks) {
		// --- ブロックごとの時間（開始遅延を考慮）---
		if (m_elapsed > block.delay && block.alpha > 0.0f) {
			// 0..1 に正規化した進行度
			float t = (m_elapsed - block.delay) / (m_duration * kFadeDurationRatio);
			t = std::clamp(t, 0.0f, 1.0f);

			// --- アルファ更新 ---
			block.alpha = std::clamp(1.0f - EaseInOut(t), 0.0f, 1.0f);

			auto color = block.sprite->GetColor();
			color.w = block.alpha;
			block.sprite->SetColor(color);

			// --- 見た目（拡縮・回転）更新 ---
			// 消失時：等倍から縮小しつつ回転
			const float eased = EaseInOut(t);
			const float disappearScale = m_disappearScaleMax - (m_disappearScaleMax - m_disappearScaleMin) * eased;
			const float disappearRot = m_disappearRotMax * eased;
			block.sprite->SetSize({m_blockSize * disappearScale, m_blockSize * disappearScale});
			block.sprite->SetRotation(disappearRot);
		}

		// --- 完了判定 ---
		if (block.alpha > 0.0f) {
			allDisappeared = false;
		}

		// 変更を描画に反映
		block.sprite->Update();
	}

	// --- フェーズ完了 ---
	if (allDisappeared) {
		m_phase = Phase::Finished;
	}
}

float SceneChangeAnimation::EaseInOut(float t) {
	// S字イージング（0..1 → 0..1）
	return (t < 0.5f) ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}

void SceneChangeAnimation::Draw() const {
	// alpha==0 のブロックは描画を省略
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

	// UI から変更したい値は一度ローカルに取り、確定時にメンバへ反映する。
	static int blockSize = m_blockSize;
	static float duration = m_duration;
	static char texturePath[128];
	strcpy_s(texturePath, sizeof(texturePath), m_blockTexturePath.c_str());

	bool needReinit = false;

	// --- 基本パラメータ ---
	needReinit |= ImGui::InputInt("Block Size", &blockSize);
	needReinit |= ImGui::InputFloat("Duration", &duration);
	needReinit |= ImGui::InputText("Texture", texturePath, sizeof(texturePath));

	ImGui::Separator();

	// --- 出現パラメータ ---
	ImGui::Text("Appear Animation");
	ImGui::SliderFloat("Appear Scale Min", &m_appearScaleMin, 0.1f, 1.0f);
	ImGui::SliderFloat("Appear Scale Max", &m_appearScaleMax, 0.1f, 2.0f);
	ImGui::SliderFloat("Appear Rot Max", &m_appearRotMax, 0.0f, 2.0f);

	// --- 消失パラメータ ---
	ImGui::Text("Disappear Animation");
	ImGui::SliderFloat("Disappear Scale Min", &m_disappearScaleMin, 0.1f, 1.0f);
	ImGui::SliderFloat("Disappear Scale Max", &m_disappearScaleMax, 0.1f, 2.0f);
	ImGui::SliderFloat("Disappear Rot Max", &m_disappearRotMax, 0.0f, 2.0f);

	// --- 再初期化 ---
	needReinit |= ImGui::Button("Reinitialize");

	if (needReinit) {
		m_blockSize = blockSize;
		m_duration = duration;
		m_blockTexturePath = texturePath;

		// 画面遷移演出の見た目が変わるため、ブロックを再生成
		Initialize();
	}

	// --- 状態表示 ---
	ImGui::Text("Phase: %s",
		m_phase == Phase::Appearing ? "Appearing" : m_phase == Phase::Disappearing ? "Disappearing" : "Finished");
	ImGui::Text("IsFinished: %s", IsFinished() ? "Yes" : "No");

	ImGui::End();
#endif // USE_IMGUI
}

void SceneChangeAnimation::SetPhase(Phase phase) {
	// フェーズ切り替え時は時間をリセットしてブロック初期状態を作り直す。
	m_phase = phase;
	m_elapsed = 0.0f;
	InitializeBlocks();
}
