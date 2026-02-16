#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include "IParticleEmitter.h"

/**
 * @brief パーティクル（エミッター）を一括管理するマネージャ。
 *
 * @details
 * `ParticleManager` は複数の `IParticleEmitter` を所有し、
 * - 生成（テンプレート/レジストリ経由）
 * - 毎フレーム更新（`Update()`）
 * - 描画（`Draw()`）
 * - シリアライズ（Save/Load）
 * - 編集支援（ImGui / Undo-Redo / Preview）
 * をまとめて提供します。
 *
 * @note
 * `CreateEmitter()` で生成したエミッターの寿命はマネージャが管理します。
 * 返されるポインタは所有権を持たない参照であり、`Remove()` や `Finalize()` 後は無効になります。
 */
class ParticleManager {
public:
    static ParticleManager* GetInstance() {
        static ParticleManager inst;
        return &inst;
    }

    ~ParticleManager() { Finalize(); }

    /**
     * @brief 全エミッターを更新します。
     * @param dt デルタタイム（秒）。
     * @param defaultCam エミッター側でカメラ未指定の場合に使用するデフォルトカメラ。
     *
     * @details
     * - 各 `IParticleEmitter` の生成/寿命/アニメーション等を更新します。
     * - Live Preview が有効な場合はプレビュー用エミッターの更新も行います。
     */
    void Update(float dt, Camera* defaultCam);

    /**
     * @brief パーティクルを描画します。
     *
     * @details
     * - 登録されている全エミッターを描画します。
     * - Live Preview が有効な場合はプレビュー用エミッターも描画します。
     *
     * @note
     * 描画対象/描画順は内部のエミッター配列の順序に依存します。
     */
    void Draw();

    void DrawImGui();

    // Registry-based creation
    IParticleEmitter* CreateEmitterByType(const std::string& typeName, const ParticlePreset& preset);

    /**
     * @brief 指定した型のエミッターを生成して登録します。
     * @tparam EmitterT `IParticleEmitter` 派生型。
     * @param preset 生成に使用するプリセット。
     * @return 生成されたエミッターへの非所有ポインタ。
     *
     * @details
     * - `preset.name` が空の場合は "Emitter" をベースにユニーク名を自動付与します。
     * - 生成後に `EmitterT::Initialize()` を呼び出します。
     * - 生成したエミッターは内部コンテナに保持され、`Finalize()` まで生存します。
     */
    template<typename EmitterT>
    EmitterT* CreateEmitter(const ParticlePreset& preset) {
        ParticlePreset adjusted = preset;
        adjusted.name = GenerateUniqueName(adjusted.name.empty() ? "Emitter" : adjusted.name);
        auto ptr = std::make_unique<EmitterT>();
        ptr->Initialize(adjusted);
        EmitterT* raw = ptr.get();
        emitters_.push_back(std::move(ptr));
        SetStatus("Created '%s'", adjusted.name.c_str());
        MarkChanged();
        return raw;
    }

    IParticleEmitter* Find(const std::string& name);
    void Remove(const std::string& name);

    void SaveAll(const std::string& filePath);
    void LoadAll(const std::string& filePath);
    void SaveSelected(const std::string& filePath, const std::vector<std::string>& names);
    void LoadMerge(const std::string& filePath);

    void Undo();
    void Redo();
    void InitialLoad(const std::string& filePath);
    void Finalize() { emitters_.clear(); previewEmitter_.reset(); history_.clear(); historyIndex_ = -1; }

private:
    ParticleManager();
    std::string GenerateUniqueName(const std::string& base) const;
    void CaptureHistory();
    void ApplySnapshot(const std::string& jsonStr);
    std::string BuildSnapshotJson() const;
    void SetStatus(const char* fmt, ...);
    void MarkChanged();
    void DrawStatusBar();
    void DrawTemplatesSection();
    void DrawEmittersSection();
    enum class PendingActionType { None, DeleteEmitter, ClearEmitter, LoadAll, LoadMergeSelected, UndoAction, RedoAction };
    PendingActionType pendingAction_ = PendingActionType::None;
    std::string       pendingEmitterName_;
    std::string       confirmMessage_;
    void OpenConfirmPopup(const char* popupName, const char* message);
    void ExecutePendingAction();

    // Preview helpers
    void UpdatePreview(float dt, Camera* cam);
    void DrawPreview(ID3D12GraphicsCommandList* cmd);
    void ApplyPreviewPreset(const ParticlePreset& src, int type);

    // Registry
    using EmitterFactoryFunc = std::function<std::unique_ptr<IParticleEmitter>()>;
    std::unordered_map<std::string, EmitterFactoryFunc> emitterRegistry_;
    void RegisterDefaultEmitters();
    std::string DetectEmitterType(IParticleEmitter* e) const;

private:
    std::vector<std::unique_ptr<IParticleEmitter>> emitters_;

    char  statusMsg_[256]{};
    float statusTimer_ = 0.0f;

    std::vector<std::string> history_;
    int  historyIndex_ = -1;
    bool changedThisFrame_ = false;

    std::string selectedEmitter_;
    bool initialLoaded_ = false;

    // プレビュー用
    std::unique_ptr<IParticleEmitter> previewEmitter_;
    int previewType_ = -1;                // 現在のプレビュー型
    ParticlePreset previewCached_;        // 前回適用した値
    bool previewEnabled_ = false;
    bool previewNeedsRecreate_ = false;
    bool previewPendingDestroy_ = false;  // LivePreview OFF時の遅延破棄
};