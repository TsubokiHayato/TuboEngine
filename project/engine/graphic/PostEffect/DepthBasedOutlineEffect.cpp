#include "DepthBasedOutlineEffect.h"
#include "ImGuiManager.h"

void DepthBasedOutlineEffect::Initialize() {
	// PSO初期化
	pso_ = std::make_unique<DepthBasedOutlinePSO>();
	pso_->Initialize();

	// 定数バッファ作成（projectionMatrix用）
	materialCB_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(DepthBasedOutlineMaterialCB));
	materialCB_->Map(0, nullptr, reinterpret_cast<void**>(&materialCBData_));
	// 初期値（単位行列）
	materialCBData_->projectionInverse = MakeIdentity4x4();


	
	// SRV作成（インデックス1にSRVを作成）
	D3D12_SHADER_RESOURCE_VIEW_DESC depthTextureSRVDesc{};
	depthTextureSRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	depthTextureSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	depthTextureSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	depthTextureSRVDesc.Texture2D.MipLevels = 1;

	DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(
	    DirectXCommon::GetInstance()->GetDepthStencliResouece().Get(), &depthTextureSRVDesc, DirectXCommon::GetInstance()->GetSRVCPUDescriptorHandle(1));
}

void DepthBasedOutlineEffect::Update() {
	// 必要に応じてprojectionMatrixを更新
	// 例: カメラや画面サイズに応じて行列をセット
	materialCBData_->projectionInverse = Inverse(camera_->GetProjectionMatrix());
}

void DepthBasedOutlineEffect::DrawImGui() {
	ImGui::Begin("DepthBasedOutlineEffect");
	// 必要に応じてImGuiでパラメータ調整
	ImGui::Text("Projection Inverse:");
	ImGui::Text("m[0][0]: %f", materialCBData_->projectionInverse.m[0][0]);
	ImGui::Text("m[1][1]: %f", materialCBData_->projectionInverse.m[1][1]);
	ImGui::Text("m[2][2]: %f", materialCBData_->projectionInverse.m[2][2]);
	ImGui::Text("m[3][3]: %f", materialCBData_->projectionInverse.m[3][3]);
	ImGui::Text("m[0][1]: %f", materialCBData_->projectionInverse.m[0][1]);
	ImGui::Text("m[1][0]: %f", materialCBData_->projectionInverse.m[1][0]);
	ImGui::End();
}

void DepthBasedOutlineEffect::SetMainCamera(Camera* camera) {
	// カメラの設定を行う
	camera_ = camera;
	if (camera) {
		// ここでカメラのプロジェクション行列を取得し、materialCBData_にセットする
		materialCBData_->projectionInverse = Inverse(camera->GetProjectionMatrix());
	}
}

void DepthBasedOutlineEffect::Draw(ID3D12GraphicsCommandList* commandList) {
	pso_->DrawSettingsCommon();
	commandList->SetGraphicsRootConstantBufferView(1, materialCB_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, DirectXCommon::GetInstance()->GetSRVGPUDescriptorHandle(1));
}