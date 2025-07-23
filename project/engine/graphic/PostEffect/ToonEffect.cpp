#include "ToonEffect.h"
#include "ImGuiManager.h"
#include "MT_Matrix.h"

ToonEffect::ToonEffect() = default;
ToonEffect::~ToonEffect() {
	if (toonCB_ && toonParams_) {
		toonCB_->Unmap(0, nullptr);
		toonParams_ = nullptr;
	}
}
void ToonEffect::Initialize() {
	pso_ = std::make_unique<ToonPSO>();
	pso_->Initialize();

	// 定数バッファ作成
	toonCB_ = DirectXCommon::GetInstance()->CreateBufferResource((sizeof(ToonParams) + 255) & ~255);
	toonCB_->Map(0, nullptr, reinterpret_cast<void**>(&toonParams_));
	// デフォルト値
	toonParams_->stepCount = 3;
	toonParams_->projectionInverse = MakeIdentity4x4();
	
	// SRV作成（インデックス1にSRVを作成）
	D3D12_SHADER_RESOURCE_VIEW_DESC depthTextureSRVDesc{};
	depthTextureSRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	depthTextureSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	depthTextureSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	depthTextureSRVDesc.Texture2D.MipLevels = 1;

	DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(
	    DirectXCommon::GetInstance()->GetDepthStencliResouece().Get(), &depthTextureSRVDesc, DirectXCommon::GetInstance()->GetSRVCPUDescriptorHandle(1));
}

void ToonEffect::Update() {
	// 必要に応じてprojectionMatrixを更新
	// 例: カメラや画面サイズに応じて行列をセット
	toonParams_->projectionInverse = Inverse(camera_->GetProjectionMatrix());
}

void ToonEffect::DrawImGui() {

	ImGui::Begin("ToonEffect");
	ImGui::SliderInt("Step Count", &toonParams_->stepCount,2, 100);
	
	ImGui::End();
}
void ToonEffect::Draw(ID3D12GraphicsCommandList* commandList) {
	pso_->DrawSettingsCommon();
	// SRV等のセットはマネージャ側で
	commandList->SetGraphicsRootConstantBufferView(1, toonCB_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, DirectXCommon::GetInstance()->GetSRVGPUDescriptorHandle(1));
}

void ToonEffect::SetMainCamera(Camera* camera) {

// カメラの設定を行う
	camera_ = camera;
	if (camera) {
		// ここでカメラのプロジェクション行列を取得し、materialCBData_にセットする
		toonParams_->projectionInverse = Inverse(camera->GetProjectionMatrix());
	}
}
