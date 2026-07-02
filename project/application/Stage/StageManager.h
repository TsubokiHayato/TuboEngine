#pragma once
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include"DirectXCommon.h"
#include "Vector3.h"
#include "engine/Collider/CollisionManager.h"

class IParticleEmitter;

class Player;
class Enemy;
class Block;
class Tile;
class MapChipField;
class FollowTopDownCamera;

namespace TuboEngine {
    class TextObject;
}

/// <summary>
/// 1ステージの矩形範囲（ワールド座標の左右下上）。StageScene::StageBounds と同等の定義。
/// </summary>
struct StageBounds {
    float left{};
    float right{};
    float bottom{};
    float top{};
};

/// <summary>
/// 複数ステージ（チャンク）の読み込み・配置・表示切替・進行を統括するクラス。
/// </summary>
class StageManager {
public:
    /// <summary>
    /// 1ステージ分のマップチップ・ブロック・敵・タイルなどをまとめて保持するデータ。
    /// </summary>
    struct StageInstance {
        std::string csvPath;
        TuboEngine::Math::Vector3 origin{0.0f, 0.0f, 0.0f};
        bool visible = true;

        std::unique_ptr<MapChipField> field;
        std::vector<std::unique_ptr<Block>> blocks;
        std::vector<std::unique_ptr<Enemy>> enemies;
        std::vector<std::unique_ptr<Tile>> tiles;

        int playerMapX = -1;
        int playerMapY = -1;

        StageBounds boundsWorld{};

        bool isCleared = false;
        float clearAnimationT = 0.0f;
    };

public:
    /// <summary>
    /// コンストラクタ。
    /// </summary>
    StageManager() = default;
    /// <summary>
    /// デストラクタ。
    /// </summary>
    ~StageManager();

    /// <summary>
    /// 動作パラメータを設定する。
    /// </summary>
    void Configure(uint32_t chunkWidth, uint32_t chunkHeight, float tileScale);

    // プレイヤーポインタを借りる（所有はしない）
    void SetPlayer(Player* player) { player_ = player; }
    Player* GetPlayer() const { return player_; }

    // チャンクIDと使用するCSVパスを登録
    void RegisterChunkCsv(int id, const std::string& path) { idToCsvPath_[id] = path; }

    /// <summary>
    /// MetaLayout の読み込み。
    /// </summary>
    void LoadMetaLayout(const std::string& metaCsvPath,
                        Player* player,
                        FollowTopDownCamera* followCamera);

    /// <summary>
    /// 更新処理。
    /// </summary>
    void Update(Player* player, FollowTopDownCamera* followCamera);

    // チェックポイント（最後にクリアしたチャンク）をリセット
    static void ResetCheckpoint();
    static int GetLastClearedChunkIndex() { return sLastClearedChunkIndex; }

    // リスタートメッセージの表示予約
    static void SetShowRestartMessage(bool show) { sShouldShowRestartMessage = show; }

    // 現在メインとなっているチャンクのインデックスを取得
    int GetMainChunkIndex() const { return mainChunkIndex_; }
    // 次のチャンクに進める（単純に mainChunkIndex_ を +1 する）
    bool AdvanceToNextChunk();

    // 全チャンクの敵が全滅しているかを判定
    bool AreAllEnemiesDefeated() const;

    /// <summary>
    /// 3Dオブジェクト描画。
    /// </summary>
    void Draw3D();

    /// <summary>
    /// PlayerStartPosition を取得する。
    /// </summary>
    TuboEngine::Math::Vector3 GetPlayerStartPosition() const;
    MapChipField* GetPlayerStartField() const; // プレイヤー開始マスの属する Field

    /// <summary>
    /// 当たり判定を衝突マネージャに登録する。
    /// </summary>
    void RegisterCollisions(CollisionManager* collisionManager,
                            Player* player);

    /// <summary>
    /// StageInstances を取得する。
    /// </summary>
    const std::vector<StageInstance>& GetStageInstances() const { return stageInstances_; }

    // ImGui で各チャンクの情報を表示
    void DrawImGui();

    /// <summary>
    /// デバッグ用: チャンクごとの敵数・撃破数の情報。
    /// </summary>
    struct ChunkEnemyInfo {
        int total = 0;
        int alive = 0;
    };
    /// <summary>
    /// ChunkEnemyInfos を取得する。
    /// </summary>
    std::vector<ChunkEnemyInfo> GetChunkEnemyInfos() const;

private:
    /// <summary>
    /// ChunkFromId の生成。
    /// </summary>
    void CreateChunkFromId(int id, int row, int col,
                           Player* player,
                           FollowTopDownCamera* followCamera);

    /// <summary>
    /// チャンク番号から配置原点を計算する。
    /// </summary>
    TuboEngine::Math::Vector3 ComputeOriginForChunk(int row, int col) const;

    /// <summary>
    /// ObjectsForChunk を構築する。
    /// </summary>
    void BuildObjectsForChunk(StageInstance& inst,
                              Player* player,
                              FollowTopDownCamera* followCamera,
                              int instanceIndex);

	// Entrance/Exit 表示用のパーティクルを更新
	void UpdateEntranceExitEffects();

private:
    std::vector<StageInstance> stageInstances_;

    uint32_t chunkWidth_  = 100;
    uint32_t chunkHeight_ = 100;
    float    tileScale_   = 30.0f;

    // チャンク間隔調整係数（1.0 でぴったり、0.5 で半分など）
    float gapScale_ = 0.5f;

    int mainChunkIndex_ = 0;

    // ID -> CSV パスの対応表
    std::unordered_map<int, std::string> idToCsvPath_;

    // StageScene から借りるだけのプレイヤー参照（所有しない）
    Player* player_ = nullptr;
    // フォローカメラへの参照
    FollowTopDownCamera* followCamera_ = nullptr;

	// Entrance / Exit 演出
	std::vector<IParticleEmitter*> entranceEmitters_;
	std::vector<IParticleEmitter*> exitEmitters_;
	int effectChunkIndex_ = -1;

	// インスタンス描画用のバッファがコマンドリスト終了まで解体されないように保持する。
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> temporaryBuffers_;

    float globalTimer_ = 0.0f;

    // セーブメッセージ用
    TuboEngine::TextObject* saveMessageText_ = nullptr;
    float saveMessageTimer_ = 0.0f;

    // 最後にクリアしたチャンクのインデックス（静的変数でシーンを跨いで保持）
    static int sLastClearedChunkIndex;
    // リスタート時にメッセージを表示するかどうかのフラグ
    static bool sShouldShowRestartMessage;
};
