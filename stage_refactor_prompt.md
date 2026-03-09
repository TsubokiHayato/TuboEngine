# ステージ管理クラス（StageManager）へのリファクタリング用プロンプト

## 1. 目的
現在の `StageScene` や `StageReadyState` に分散しているステージ読み込み・管理ロジックを、新しく作成する `StageManager` クラス（または `StageLayoutLoader`）に集約し、メタ・レイアウト（`Stage.csv`）に基づいた柔軟なステージ構築を可能にします。

## 2. クラス設計（StageManager）

### メンバ変数
- `std::vector<StageInstance> stageInstances_`: 各ステージチャンク（CSV）のデータ。
    - `StageInstance` には `MapChipField`, `Block`, `Enemy` のリストと、ワールド座標上の `Origin` を含める。
- `uint32_t chunkWidth_, chunkHeight_`: 各チャンクの固定サイズ（タイル数）。
- `float tileScale_`: タイル1枚のサイズ（ワールド座標単位）。

### メソッド
- `void LoadMetaLayout(const std::string& metaCsvPath)`: 
    - `Stage.csv` を読み込み、各セルの数字 `N` に応じて `StageN.csv` をしかるべき位置（Origin）に配置・ロードする。
- `void Update()`: 
    - `StageInstance` 全体の更新（敵のAI進行など）を回す。
- `void Draw(Camera* camera)`:
    - 描画処理を一括で行う。
- `Vector3 GetPlayerStartPosition()`: 
    - 全チャンクの中から `Player` チップ（ID=2）を検索し、そのワールド座標を返す。
- `void RegisterCollisions(CollisionManager* collisionManager)`:
    - 全チャンクのオブジェクトを衝突マネージャに登録する。

## 3. 処理の流れ

1.  **メタ・ロード**:
    - `Stage.csv` を走査。`(row, col) = ID` を取得。
    - `ID > 0` ならば `Stage{ID}.csv` をロード。
    - 座標計算: `Origin.x = col * chunkWidth_ * tileScale_`, `Origin.y = -row * chunkHeight_ * tileScale_` (配置順は適宜調整)。
2.  **インスタンス化**:
    - `MapChipField` を各チャンクに持たせ、チップの種類に応じて `Block` や `Enemy` をそのチャンク専用のリストに生成する。
3.  **統合**:
    - `StageScene` は `StageManager` のインスタンスを1つ持ち、`Update/Draw` を委譲するだけで済むようにする。

## 4. 特記事項
- ステージチャンク同士が重ならないよう、`Stage.csv` のグリッド座標から正確なワールド座標を導出すること。
- ロード済みの `StageInstance` を `visible` フラグ等で管理し、プレイ中の領域外チップの更新をスキップするなどの最適化も視野に入れる。
