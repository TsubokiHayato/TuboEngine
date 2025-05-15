//#pragma once
//#include <d3d12.h>
//#include <wrl.h>
//#include <memory>
//#include "DirectXCommon.h"
//
//class OffscreenRenderering
//{
//public:
//    OffscreenRenderering();
//    ~OffscreenRenderering();
//
//    // 初期化
//    void Initialize(DirectXCommon* dxCommon, int width, int height);
//
//    // オフスクリーン描画開始（RTV/クリア/バリア）
//    void Begin();
//
//    // オフスクリーン描画終了（バリアをSRVへ）
//    void End();
//
//    // 全画面三角形コピー
//    void DrawCopy(ID3D12PipelineState* pso, ID3D12RootSignature* rootSig, D3D12_GPU_DESCRIPTOR_HANDLE srvHandle);
//
//    // オフスクリーンテクスチャのSRVハンドル取得
//    D3D12_GPU_DESCRIPTOR_HANDLE GetSRVHandle() const;
//
//    // オフスクリーンテクスチャリソース取得
//    ID3D12Resource* GetResource() const;
//
//private:
//    DirectXCommon* dxCommon_ = nullptr;
//    Microsoft::WRL::ComPtr<ID3D12Resource> renderTexture_;
//    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_{};
//    D3D12_GPU_DESCRIPTOR_HANDLE srvHandle_{};
//    int width_ = 0;
//    int height_ = 0;
//};
