#include "randomEffect.h"
#include "ImGuiManager.h"
#include <random>
#include <chrono>

randomEffect::randomEffect() = default;
randomEffect::~randomEffect() {
    if (cbResource_ && params_) {
        cbResource_->Unmap(0, nullptr);
        params_ = nullptr;
    }
}

void randomEffect::Initialize(DirectXCommon* dxCommon) {
    // PSO初期化
    pso_ = std::make_unique<randomPSO>();
    pso_->Initialize(dxCommon);
    // 定数バッファ作成
    cbResource_ = dxCommon->CreateBufferResource(sizeof(RandomParams));
    cbResource_->Map(0, nullptr, reinterpret_cast<void**>(&params_));

    // 乱数エンジンで値を生成（シードは時間ベースで初期化）
    std::random_device rd;
    std::mt19937 randomEngine(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
   
    params_->time = 0.0f; // timeはUpdateで進める
	params_->randomValue = dist(randomEngine); // 初期値を設定
	params_->padding[0] = 0.0f; // 16バイトアライメントのためのパディング
	params_->padding[1] = 0.0f; // 16バイトアライメントのためのパディング
}


void randomEffect::Update() {
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - start).count();
    params_->time = elapsed;

    // 乱数生成
    static std::mt19937 engine{ std::random_device{}() };
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    params_->randomValue = dist(engine);
}


void randomEffect::DrawImGui() {
    ImGui::Begin("Random Effect");
    // timeは自動で進むので表示のみ
    ImGui::Text("Time: %.2f", params_->time);
   
    ImGui::End();
}

void randomEffect::Draw(ID3D12GraphicsCommandList* commandList) {
    // PSO・ルートシグネチャ設定
    pso_->DrawSettingsCommon();
    // CBVをバインド
    //commandList->SetGraphicsRootConstantBufferView(1, cbResource_->GetGPUVirtualAddress());
}
