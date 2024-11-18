#pragma once

class DirectXCommon; // 前方宣言

class ModelCommon
{
public:
    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="dxCommon">DirectX共通部分</param>
    void Initialize(DirectXCommon* dxCommon);

    /// <summary>
    /// DirectX共通部分を取得する
    /// </summary>
    /// <returns>DirectXを返す</returns>
    DirectXCommon* GetDxCommon() { return dxCommon_; }
private:
    // DirectX共通部分
    DirectXCommon* dxCommon_ = nullptr;
};
