# エンジン / アプリケーション分離 設計書

- 作業ブランチ: `Ver20.0_feature/EngineSparation`（※名称タイポ "Separation" 要確認）
- 起点: `Ver20.0_feature/DebugCamera`（※Develop基点に変えるか要確認）
- ステータス: ドラフト（チームレビュー待ち。コード未変更）
- 最終更新: 2026-06-15

---

## 1. 目的・ゴール

TuboEngine を「**このゲームを一切知らない、再利用可能なエンジン**」と「**エンジンを使う側のゲーム（application）**」に分離する。

完了条件（Definition of Done）:

1. `project/engine/` 配下のどのファイルも `project/application/` を `#include` していない（依存が一方向）。
2. エンジンは「どんなシーンが存在するか」を一切知らない（`TitleScene` 等の具体名・`STAGE` 等の enum を持たない）。
3. ゲーム側は今まで通り起動・動作する（挙動の変化なし、純粋なリファクタ）。
4. 「別のゲームを作るとき、`application/` を差し替えるだけでエンジンが使い回せる」状態。

### 分離の3レベル（用語整理）

| Lv | 分離の種類 | 手段 | このドキュメントでの扱い |
|----|-----------|------|------------------------|
| 1 | 論理（フォルダ／コード） | `engine/` と `application/` の依存を一方向化 | §3〜§6 フェーズA〜C（最優先） |
| 2 | ビルド（プロジェクト） | `Engine.lib` + `Game.exe` に分割 | §6 フェーズD（最終ゴール） |
| 3 | リポジトリ（GitHub） | 別リポジトリへ切り出す | 当面やらない（チーム制作中は monorepo 維持） |

> このドキュメントは**設計のみ**。実装は別途、フェーズ単位で進める（§6）。

---

## 2. 現状診断

### 2.1 良い状態（既にクリーン）

エンジンの大部分はゲームを知らない。フォルダ分けも済んでいる。

| 領域 | 状態 |
|------|------|
| `graphic` / `base` / `audio` / `input` / `math` / `camera` / `imgui` / `leakChecker` | ✅ ゲーム非依存。触らない |
| `scene/IScene`・`scene/SceneManager`（仕組み部分） | △ 仕組みは汎用だがゲーム知識が混入（§2.2） |

### 2.2 問題：`engine/scene/` でのみ依存が逆流している

エンジン → ゲームの矢印が出ている箇所は次の4つだけ。

```
engine/Framework ──► SceneManager ──► TitleScene/StageScene/... ──► application/Player,Enemy,UI...
   (engine)            (engine)           (本来ゲーム側)                    (ゲーム)
                                  ▲──────── ここが逆流 ────────▲
```

| # | 場所 | 問題 |
|---|------|------|
| 1 | `engine/scene/IScene.h:9` | `enum SCENE{DEBUG,TITLE,STAGE,TUTORIAL,CLEAR,OVER}` ＝ ゲームのシーン名がエンジン基底に埋まっている |
| 2 | `engine/scene/SceneManager.cpp:3-7` | 具体シーンを直 `#include` し `make_unique<StageScene>()` で生成（ファクトリがゲームを知っている） |
| 3 | `engine/scene/{Title,Stage,Clear,Over,Debug}Scene.*` | ゲーム専用シーンがエンジン内に居住。これらが `application/Character/Player`・`Bullet`・`UI`・`StageManager` を `#include` |
| 4 | `engine/Framework/Framework.cpp:64` | `SceneManager::...->Initialize(STAGE)` ＝ 開始シーンをエンジンがハードコード |

加えて `SceneManager::ImGuiDraw()`（`SceneManager.cpp:118-150`）の「Debug / Title / Stage / Tutorial / Clear / Over」ボタンもゲーム固有。

### 2.3 影響範囲（実測）

- `SCENE` enum 値の参照: **52ヶ所 / 18ファイル**
  - アプリ側: `application/StageState/` 配下の各 State（`StagePlayingState` `PauseState` `GameOverState` `GameClearState` `StageClearState` `StageTransitionState` `TutorialState`）がシーン遷移で使用
  - エンジン側: `Framework.cpp` と `scene/` の具体シーン群
- `SceneManager` 参照: アプリ3ファイル（`StageClearState` `StagePlayingState` `StageTransitionState`）＋ エンジン側

---

## 3. 設計方針：依存性の逆転（シーン登録制ファクトリ）

エンジンは「`IScene` を生成する関数」を受け取るだけにする。**どの番号がどのシーンか**はゲーム側が起動時に登録する。

```
[Before] SceneManager が具体シーンを知っている（エンジン→ゲーム）
[After]  ゲームが起動時に「番号→生成関数」を SceneManager に登録（ゲーム→エンジン）
```

### 3.1 エンジン側 API（`engine/scene/`）

**`IScene.h`**: `enum SCENE` を削除。シーン番号は意味を持たない `int` のまま扱う（番号の意味はゲームが所有）。インターフェイスは現状維持。

**`SceneManager`**: ハードコードの if-else を「登録テーブル」に置き換える。

```cpp
// SceneManager.h（設計イメージ）
#include <functional>
#include <unordered_map>
#include <string>
#include <memory>
#include "IScene.h"

class SceneManager {
public:
    using SceneFactory = std::function<std::unique_ptr<IScene>()>;

    // ゲームが起動時に呼ぶ：番号→生成関数を登録
    void RegisterScene(int sceneNo, SceneFactory factory, const std::string& debugName = "");

    // 開始シーン番号を指定して開始（登録済みの番号であること）
    void Initialize(int startSceneNo);

    void Update();
    void Finalize();
    void ChangeScene(int sceneNo);
    // 描画系・GetMainCamera は現状維持
    // ...

private:
    std::unique_ptr<IScene> CreateScene(int sceneNo); // テーブル参照して生成

    std::unordered_map<int, SceneFactory>  factories_;
    std::unordered_map<int, std::string>   debugNames_; // ImGui用（任意）
    // currentScene_ / currentSceneNo_ などは現状維持
};
```

- `Initialize` / `Update` 内の `if (no==TITLE) make_unique<TitleScene>()...` は **すべて `CreateScene(no)`（テーブル参照）に置換**。
- `ImGuiDraw()` のシーン選択ボタンは `debugNames_` を回してデータ駆動で生成（ハードコードのボタンを撤去）。

> `int` キーで進める（既存コードが int 前提のため改修が最小）。将来的に文字列キーにしたくなったら拡張可能。

### 3.2 ゲーム側（`application/scene/` を新設）

**`GameScenes.h`**（ゲーム固有のシーン番号）:
```cpp
#pragma once
enum GameScene { DEBUG, TITLE, STAGE, TUTORIAL, CLEAR, OVER };
```
> 値の並びは現行 `enum SCENE` と同一にする（既存の遷移ロジックを壊さないため）。

**具体シーンの移動**: `Title/Stage/Clear/Over/DebugScene` を `engine/scene/` → `application/scene/` へ移設。

**`SceneRegistration.{h,cpp}`**（登録の入口）:
```cpp
// SceneRegistration.cpp（設計イメージ）
void RegisterGameScenes() {
    auto* sm = SceneManager::GetInstance();
    sm->RegisterScene(DEBUG,    []{ return std::make_unique<DebugScene>();  }, "Debug");
    sm->RegisterScene(TITLE,    []{ return std::make_unique<TitleScene>();  }, "Title");
    sm->RegisterScene(STAGE,    []{ return std::make_unique<StageScene>();  }, "Stage");
    sm->RegisterScene(TUTORIAL, []{ return std::make_unique<StageScene>();  }, "Tutorial"); // 現行同様 StageScene を流用
    sm->RegisterScene(CLEAR,    []{ return std::make_unique<ClearScene>();  }, "Clear");
    sm->RegisterScene(OVER,     []{ return std::make_unique<OverScene>();   }, "Over");
}
```

### 3.3 起動フローの付け替え

現状 `Framework::Initialize()`（エンジン）が `SceneManager::Initialize(STAGE)` を呼んでいる（§2.2 #4）。これを **ゲーム側（`Order`）に移す**。

```cpp
// Order::Initialize()（ゲーム側、設計イメージ）
void TuboEngine::Order::Initialize() {
    TuboEngine::Framework::Initialize();   // エンジン基盤の初期化（シーンには触れない）
    RegisterGameScenes();                  // ゲームのシーンを登録
    SceneManager::GetInstance()->Initialize(STAGE); // 開始シーンを指定
}
```

- `Framework::Initialize()` からは `SceneManager::Initialize(STAGE)` の行を**削除**（エンジンは開始シーンを決めない）。
- `Framework::Update()/Draw()` 内の `SceneManager::GetInstance()->Update()/Draw()` 等は**汎用なので据え置き**（具体シーンを知らないため問題なし）。

> **補足**: `Order`（`engine/Framework/Order.*`）は実質「ゲームの起動クラス」なので、本来は `application/` 側に置くのが筋。ただし移動は `main.cpp` の include 変更を伴うため、フェーズで切り分ける（§6 フェーズC）。

---

## 4. フォルダ構成 Before / After

```
Before                                    After
engine/scene/                             engine/scene/
├ IScene.{h,cpp}        ← enum混在        ├ IScene.{h,cpp}        ← 純粋IF（enum削除）
├ SceneManager.{h,cpp}  ← 具体依存        ├ SceneManager.{h,cpp}  ← 登録制ファクトリ
├ SceneType.h                             └ SceneType.h（必要なら整理）
├ TitleScene.{h,cpp}    ─┐
├ StageScene.{h,cpp}     │ ゲーム固有     application/scene/         ← 新設
├ ClearScene.{h,cpp}     │  →移動         ├ GameScenes.h           ← enum GameScene
├ OverScene.{h,cpp}      │                ├ SceneRegistration.{h,cpp}
└ DebugScene.{h,cpp}    ─┘                ├ TitleScene.{h,cpp}
                                          ├ StageScene.{h,cpp}
                                          ├ ClearScene.{h,cpp}
                                          ├ OverScene.{h,cpp}
                                          └ DebugScene.{h,cpp}
```

`.vcxproj` の `AdditionalIncludeDirectories` に `application\scene` を追加し、各シーンファイルの `<ClCompile>/<ClInclude>` のパスを更新する（フィルタも合わせる）。

---

## 5. 修正が必要なファイル一覧

| 区分 | ファイル | 作業 |
|------|----------|------|
| エンジン | `engine/scene/IScene.h` | `enum SCENE` 削除 |
| エンジン | `engine/scene/SceneManager.{h,cpp}` | 登録制に改修、具体 include 撤去、ImGui ボタンをデータ駆動化 |
| エンジン | `engine/Framework/Framework.cpp` | `Initialize(STAGE)` の行を削除（開始指定をゲームへ） |
| 移動 | `engine/scene/{Title,Stage,Clear,Over,Debug}Scene.*` | `application/scene/` へ移設 + include パス調整 |
| ゲーム新規 | `application/scene/GameScenes.h` | enum 定義 |
| ゲーム新規 | `application/scene/SceneRegistration.{h,cpp}` | 登録処理 |
| ゲーム | `engine/Framework/Order.cpp`（→将来 application へ） | 登録呼び出し + 開始シーン指定 |
| ゲーム | `application/StageState/*State.cpp`（7ファイル） | `enum SCENE` を `#include "GameScenes.h"` 経由に切替（値は同じなので置換は include 追加が主） |
| ビルド | `project/TuboEngine.vcxproj` / `.filters` | ファイル移動とインクルードパス反映 |

---

## 6. 移行手順（フェーズ分け）

各フェーズ末で**必ずビルド＆起動確認**。1フェーズ＝1コミット推奨。

### フェーズA: エンジンを「登録制」対応にする（後方互換を保ったまま）
1. `SceneManager` に `RegisterScene` / 内部テーブル / `CreateScene` を追加（既存 if-else は残したまま）。
2. ビルド確認（この時点では挙動不変）。

### フェーズB: 具体シーンをゲーム側へ移し、登録に切替
3. `Title/Stage/Clear/Over/DebugScene` を `application/scene/` へ移動、`.vcxproj` 更新。
4. `GameScenes.h` と `SceneRegistration.{h,cpp}` を新設。
5. `Order::Initialize()` で `RegisterGameScenes()` + `SceneManager::Initialize(START)` を呼ぶ。
6. `SceneManager` から具体 `#include` と if-else を撤去（`CreateScene` のテーブル参照に一本化）。`ImGuiDraw` をデータ駆動化。
7. `Framework.cpp` の `Initialize(STAGE)` を削除。
8. `IScene.h` の `enum SCENE` を削除。`application/StageState/*` 各 State に `#include "GameScenes.h"` を追加。
9. ビルド＆起動確認（挙動不変であること）。

### フェーズC（任意・仕上げ）: 起動クラスの所在を整理
10. `Order.*` を `application/` 配下へ移動し、`main.cpp` の include を更新（エンジンに残る Framework 派生のゲームクラスを無くす）。
11. 最終検証（§9）。

### フェーズD（最終ゴール・任意）: プロジェクト物理分割（engine の .lib 化）
> ①〜③（A〜C）が完了し、`grep` で依存ゼロを確認してから着手する。`.vcxproj` を大きく書き換えるためマージコンフリクトが起きやすい。**チーム全員に周知し、他の作業が落ち着いたタイミングで一気にやる**こと。

現状: `TuboEngine.vcxproj` は `ConfigurationType=Application`（.exe）1個に engine + application + main + externals/imgui を全部入れている。これを2プロジェクトに割る。

```
[Before] TuboEngine.exe  ← engine + application + main を1プロジェクトでビルド
[After]  Engine.lib      ← engine/ を静的ライブラリ化（application を一切知らない）
         Game.exe        ← application/ + main、Engine.lib をリンク
```

手順:
1. 新規 `Engine.vcxproj`（`ConfigurationType=StaticLibrary`）を作り、`engine/` 配下のソースと include パスを移す。externals（imgui/DirectXTex/assimp）の扱いを決める（エンジン側に寄せる）。
2. 既存 `TuboEngine.vcxproj` は application + main だけを残し、`Engine.vcxproj` への **プロジェクト参照（References）** を追加。エンジンの公開ヘッダを include できるようパスを通す。
3. ソリューションのビルド順（Engine → Game）を依存関係で自動解決させる。
4. ビルド＆起動確認。

**ねらい**: ここで初めて境界がコンパイラに強制される。エンジンからゲームを `#include` したら**ビルドエラー**になり、二度と逆流できなくなる。これが「本当の分離」。§7 で挙げる「エンジン昇格候補」は、このフェーズで engine 側に置く。

> リポジトリ（GitHub）分割は Lv3 でさらにその先。チーム制作中は monorepo 維持を推奨（submodule 運用コストが高く、エンジンとゲームの同時変更がアトミックにできなくなるため）。再利用は本フェーズD（`Engine.lib` リンク）でほぼ達成できる。

---

## 7. アプリ→エンジン昇格候補（逆方向の整理）

`application/` にあるが**実体は汎用で、エンジンに昇格させるべき**もの。これは「フェーズDで `Engine.lib` 側に置くもの」を決める作業でもある。
（現状は同一 .exe にコンパイルされるため機能差は出ない。昇格は**分離のブロッカーではない**＝シーン分離 A〜C を優先。）

### ◎ そのまま昇格OK（汎用・低リスク）
| 対象 | 依存 | 理由 |
|------|------|------|
| `application/BT/BehaviorTree.h` | STLのみ | ヘッダオンリーの汎用AIフレームワーク。即移動可 |
| `application/Collider/Collider.{h,cpp}` + `CollisionManager.{h,cpp}` | Vector3 のみ | 球判定・コライダー管理＝完全汎用な当たり判定基盤 |
| `application/Animation/SceneChangeAnimation.{h,cpp}` | Sprite(engine) のみ | 画面遷移エフェクト。ゲーム非依存 |

> ⚠ `application/Collider/CollisionTypeId.h`（kPlayer/kEnemy…）は**ゲーム固有なのでアプリに残す**。Collider 本体は `uint32_t typeID_` で扱い enum に依存していないため、キレイに分離できる。

### △ 昇格できるが「脱結合」が先に必要（概念は汎用、実装がゲーム依存）
| 対象 | 引っかかり | 必要な作業 |
|------|-----------|-----------|
| `application/Camera/FollowTopDownCamera.{h,cpp}` | `Player*` に直結（`#include Player.h`・`SetTarget(Player*)`） | ターゲットを `Transform*`／位置プロバイダに抽象化すれば昇格可 |
| `application/MapChip/MapChipField.{h,cpp}` | `MapChipType` enum にゲームの敵種別(EnemyRush 等)が混入 | グリッド基盤とタイル語彙(enum)を分離すれば昇格可 |

### ❌ アプリに残す（ゲーム固有）
- `StageState/`（`StageStateManager`・`IStageState`・各 State）… `StageScene*` と `StageType` に密結合。ステージ進行専用
- `Character`（Player/Enemy 系）・`Bullet`・`UI`・`Stage`・`SkyDome`・`Block`・`Tile`
- `GameConstants.h`・`Particle/CharacterParticlePresets.h`・`Collider/CollisionTypeId.h`・`MapChip` の `MapChipType`

### 昇格の進め方
- **◎の3つ**: シーン分離（A〜C）が落ち着いたら移しておくと、フェーズDの `Engine.lib` 化が楽になる。低リスク。
- **△の2つ**: 脱結合リファクタが要るので後回し or 別タスク（別ブランチ推奨）。

---

## 8. リスク・チームへの注意

- **挙動を変えない**。これは純粋なリファクタ。シーン遷移の見た目・順序が変わったらバグ。
- **enum 値の並びを変えない**（`DEBUG=0,TITLE=1,...`）。`SetSceneNo(int)` に数値が直書きされている箇所がもしあれば壊れる。
- **シングルトンの初期化順序**: `RegisterGameScenes()` は `SceneManager::Initialize()` より**前**に呼ぶこと。
- **他メンバーの作業との衝突**: シーン追加・遷移をいじっている人がいたら、フェーズBのマージタイミングを調整。
- **`.vcxproj` のコンフリクト注意**: ファイル移動・プロジェクト分割（D）はプロジェクトファイルを書き換えるため、他の人の追加ファイルとぶつかりやすい。先に共有してから実施。

---

## 9. 完了検証

1. 依存の一方向性を機械的に確認:
   ```sh
   # エンジンが application を include していたら 0 件のはず
   grep -rnE '#include.*application' project/engine
   # エンジンに具体シーン名が残っていたら 0 件のはず
   grep -rnwE '(TitleScene|StageScene|ClearScene|OverScene|DebugScene)' project/engine
   ```
2. ビルドが通り、ゲームが従来通り起動・遷移する。
3. （理想）`application/` を空のサンプルに差し替えてもエンジンがビルドできる。
