"use strict";
const pptxgen = require("pptxgenjs");

// ── カラー定義 ─────────────────────────────────────────────────────────────
const C = {
  bg:      "111827",  // ほぼ黒（メイン背景）
  panel:   "1F2937",  // カードパネル
  panel2:  "374151",  // 薄いパネル
  accent:  "F97316",  // オレンジアクセント
  blue:    "3B82F6",  // ブルー
  green:   "10B981",  // グリーン（良い例）
  red:     "EF4444",  // レッド（悪い例・問題）
  yellow:  "FBBF24",  // 黄（警告・ポイント）
  text:    "F9FAFB",  // メインテキスト（白）
  muted:   "9CA3AF",  // サブテキスト（グレー）
  code:    "0F172A",  // コードブロック背景
  codeT:   "7DD3FC",  // コードテキスト
};

const pres = new pptxgen();
pres.layout  = "LAYOUT_16x9";
pres.author  = "Tsuboki Hayato";
pres.title   = "TuboEngine プログラム説明資料";

// ── ユーティリティ ──────────────────────────────────────────────────────────
const makeShadow = () => ({
  type: "outer", color: "000000", blur: 10, offset: 3, angle: 135, opacity: 0.25
});

// コードブロックを描画（等幅フォント風、暗いパネル）
function addCode(slide, code, x, y, w, h) {
  slide.addShape(pres.shapes.RECTANGLE, {
    x, y, w, h,
    fill: { color: C.code },
    line: { color: "1E293B", width: 1 },
    shadow: makeShadow(),
  });
  slide.addText(code, {
    x: x + 0.12, y: y + 0.07, w: w - 0.24, h: h - 0.14,
    fontFace: "Consolas", fontSize: 8.5, color: C.codeT,
    align: "left", valign: "top", margin: 0,
  });
}

// 数値コールアウト（大きい数字 + ラベル）
function addStat(slide, num, label, x, y, w = 2.0, accent = C.accent) {
  slide.addShape(pres.shapes.RECTANGLE, {
    x, y, w, h: 1.0,
    fill: { color: C.panel }, line: { color: accent, width: 2 },
    shadow: makeShadow(),
  });
  slide.addText(num, {
    x: x + 0.08, y: y + 0.04, w: w - 0.16, h: 0.52,
    fontFace: "Arial Black", fontSize: 22, color: accent,
    align: "center", bold: true, margin: 0,
  });
  slide.addText(label, {
    x: x + 0.08, y: y + 0.55, w: w - 0.16, h: 0.36,
    fontFace: "Calibri", fontSize: 9, color: C.muted,
    align: "center", margin: 0,
  });
}

// セクションラベル（左端に縦線 + 小文字ラベル）
function addSectionLabel(slide, label, x, y, color = C.accent) {
  slide.addShape(pres.shapes.RECTANGLE, {
    x, y, w: 0.06, h: 0.22,
    fill: { color }, line: { color, width: 0 },
  });
  slide.addText(label.toUpperCase(), {
    x: x + 0.12, y: y, w: 3, h: 0.22,
    fontFace: "Arial", fontSize: 8, color,
    bold: true, charSpacing: 3, margin: 0,
  });
}

// カードパネル
function addCard(slide, x, y, w, h, color = C.panel) {
  slide.addShape(pres.shapes.RECTANGLE, {
    x, y, w, h,
    fill: { color },
    line: { color: "374151", width: 1 },
    shadow: makeShadow(),
  });
}

// スライド共通のヘッダー（スライドタイトル部分）
function addSlideHeader(slide, title, subtitle = "") {
  // 上部ダーク帯
  slide.addShape(pres.shapes.RECTANGLE, {
    x: 0, y: 0, w: 10, h: 1.0,
    fill: { color: "0D1117" }, line: { color: "0D1117", width: 0 },
  });
  // アクセントバー（細い）
  slide.addShape(pres.shapes.RECTANGLE, {
    x: 0, y: 1.0, w: 10, h: 0.04,
    fill: { color: C.accent }, line: { color: C.accent, width: 0 },
  });
  slide.addText(title, {
    x: 0.4, y: 0.08, w: 8.5, h: 0.55,
    fontFace: "Arial Black", fontSize: 20, color: C.text,
    align: "left", bold: true, margin: 0,
  });
  if (subtitle) {
    slide.addText(subtitle, {
      x: 0.4, y: 0.62, w: 8.5, h: 0.30,
      fontFace: "Calibri", fontSize: 11, color: C.muted,
      align: "left", margin: 0,
    });
  }
  // 右端ページ番号エリア（スライド番号は pptx が自動で振るので省略）
}

// ── スライド 1: タイトル ────────────────────────────────────────────────────
{
  const s = pres.addSlide();
  s.background = { color: C.bg };

  // 背景装飾グリッド風の薄い線
  for (let i = 0; i < 6; i++) {
    s.addShape(pres.shapes.LINE, {
      x: 0, y: 1.1 * i + 0.5, w: 10, h: 0,
      line: { color: "1F2937", width: 0.5 },
    });
  }

  // アクセントブロック
  s.addShape(pres.shapes.RECTANGLE, {
    x: 0, y: 0, w: 0.18, h: 5.625,
    fill: { color: C.accent }, line: { color: C.accent, width: 0 },
  });

  s.addText("TuboEngine", {
    x: 0.5, y: 0.6, w: 9, h: 1.1,
    fontFace: "Arial Black", fontSize: 52, color: C.text,
    align: "left", bold: true, margin: 0,
  });
  s.addText("プログラム説明資料", {
    x: 0.5, y: 1.75, w: 9, h: 0.6,
    fontFace: "Calibri", fontSize: 22, color: C.accent,
    align: "left", bold: true, margin: 0,
  });
  s.addText("DirectX 12 / C++17 / HLSL", {
    x: 0.5, y: 2.45, w: 9, h: 0.35,
    fontFace: "Consolas", fontSize: 13, color: C.muted,
    align: "left", margin: 0,
  });

  // 右側：技術スタックバッジ風
  const badges = [
    { label: "DirectX 12",  color: C.blue   },
    { label: "C++17",        color: C.green  },
    { label: "HLSL",         color: C.yellow },
    { label: "assimp",       color: C.accent },
    { label: "Dear ImGui",   color: "A78BFA" },
  ];
  badges.forEach((b, i) => {
    s.addShape(pres.shapes.RECTANGLE, {
      x: 6.8, y: 1.5 + i * 0.55, w: 2.8, h: 0.38,
      fill: { color: C.panel }, line: { color: b.color, width: 1.5 },
      shadow: makeShadow(),
    });
    s.addText(b.label, {
      x: 6.8, y: 1.5 + i * 0.55, w: 2.8, h: 0.38,
      fontFace: "Consolas", fontSize: 11, color: b.color,
      align: "center", valign: "middle", margin: 0,
    });
  });

  s.addText("制作期間：2025年3月 〜 2026年5月（約14ヶ月）  /  Tsuboki Hayato", {
    x: 0.5, y: 5.0, w: 9, h: 0.35,
    fontFace: "Calibri", fontSize: 10, color: C.muted,
    align: "left", margin: 0,
  });
}

// ── スライド 2: 説明の構成（フレームワーク） ───────────────────────────────
{
  const s = pres.addSlide();
  s.background = { color: C.bg };
  addSlideHeader(s, "技術説明の構成", "各トピックをこの流れで説明します");

  const steps = [
    { num: "01", title: "実現したかったこと",  desc: "このゲームで直面した具体的な要件・制約",   color: C.blue   },
    { num: "02", title: "比較検討した手法",    desc: "他のアプローチとその限界（Gap）",         color: C.yellow },
    { num: "03", title: "技術選定の理由",      desc: "なぜこの手法を選んだか（必然性）",         color: C.accent },
    { num: "04", title: "実装とコード",        desc: "意図が伝わる最小限のコード断片",           color: "A78BFA" },
    { num: "05", title: "定量的な結果",        desc: "Before / After の数値、計測条件",          color: C.green  },
    { num: "06", title: "トレードオフ",        desc: "何を得て何を捨てたか（成熟度の証明）",     color: C.muted  },
  ];

  steps.forEach((st, i) => {
    const col = i < 3 ? 0 : 1;
    const row = i % 3;
    const x = 0.3 + col * 4.85;
    const y = 1.2 + row * 1.35;

    addCard(s, x, y, 4.5, 1.15);
    s.addText(st.num, {
      x: x + 0.15, y: y + 0.10, w: 0.55, h: 0.55,
      fontFace: "Arial Black", fontSize: 20, color: st.color,
      align: "center", margin: 0,
    });
    s.addShape(pres.shapes.LINE, {
      x: x + 0.78, y: y + 0.15, w: 0, h: 0.78,
      line: { color: st.color, width: 1.5 },
    });
    s.addText(st.title, {
      x: x + 0.95, y: y + 0.08, w: 3.38, h: 0.35,
      fontFace: "Calibri", fontSize: 12.5, color: C.text,
      bold: true, align: "left", margin: 0,
    });
    s.addText(st.desc, {
      x: x + 0.95, y: y + 0.45, w: 3.38, h: 0.55,
      fontFace: "Calibri", fontSize: 10, color: C.muted,
      align: "left", margin: 0,
    });
  });
}

// ── スライド 3: ホーミング弾（距離依存ターンアシスト） ─────────────────────
{
  const s = pres.addSlide();
  s.background = { color: C.bg };
  addSlideHeader(s, "ホーミング弾の「距離依存ターンアシスト」", "PlayerCircusBullet.cpp");

  // 左カラム：要件 + 比較
  addCard(s, 0.3, 1.15, 4.3, 1.4);
  addSectionLabel(s, "実現したかったこと", 0.42, 1.20, C.blue);
  s.addText([
    { text: "カウンター弾は視覚的に派手な曲線軌道を描きつつ、", options: { breakLine: true } },
    { text: "爽快感の核心なので「外れる不安」を与えてはいけない。", options: { breakLine: true } },
    { text: "→ 見た目と確実性を両立する軌道設計が必要だった。" },
  ], {
    x: 0.42, y: 1.46, w: 4.06, h: 1.0,
    fontFace: "Calibri", fontSize: 10, color: C.text,
    align: "left", margin: 0,
  });

  // 比較テーブル
  addCard(s, 0.3, 2.65, 4.3, 1.65);
  addSectionLabel(s, "比較検討した手法", 0.42, 2.70, C.yellow);
  const rows = [
    ["案", "手法", "問題点"],
    ["A", "直進弾",             "見た目が地味で演出にならない"],
    ["B", "単純ホーミング",     "遠距離でも不自然に曲がる"],
    ["C", "サイン波のみ",       "近距離で外れることがある"],
    ["D★", "サイン波＋アシスト", "遠距離は派手・近距離は確実"],
  ];
  const rowColors = [C.panel2, C.panel, C.panel, C.panel, "1A3A2A"];
  const textColors = [C.muted, C.text, C.text, C.text, C.green];
  s.addTable(rows.map((r, i) => r.map((cell, ci) => ({
    text: cell,
    options: {
      fill: { color: rowColors[i] },
      color: i === 0 ? C.muted : (i === 4 ? C.green : C.text),
      bold: i === 0 || (i === 4),
      fontFace: ci === 0 ? "Consolas" : "Calibri",
      fontSize: i === 0 ? 8 : 9,
      align: ci === 0 ? "center" : "left",
    },
  }))), {
    x: 0.42, y: 2.94, w: 4.05, h: 1.26,
    colW: [0.35, 1.2, 2.5],
    border: { pt: 0.5, color: "374151" },
  });

  // 右カラム：コード + 数値
  addSectionLabel(s, "実装（実コード抜粋）", 4.8, 1.18, C.accent);
  addCode(s,
    `// PlayerCircusBullet.cpp  ※近距離補正\nif (distanceToTarget < 6.0f) {\n    float t = distanceToTarget / 6.0f;\n    //  ↑ 0→1（ターゲット近づくにつれ0へ）\n\n    playerChaosAmp  *= t;      // カオス振幅をフェードアウト\n    playerTurnSpeed *= (2.0f - t); // ターン速度を最大 2x 強化\n}`,
    4.8, 1.42, 4.88, 1.55
  );

  // 数値コールアウト
  addStat(s, "2.2×", "ターン速度ブースト上限",  4.80, 3.1,  2.35, C.accent);
  addStat(s, "40%",  "カオス振幅の抑制率",       7.28, 3.1,  2.35, C.blue);
  addStat(s, "6u",   "距離切り替え閾値",          4.80, 4.22, 2.35, C.green);
  addStat(s, "0.4s", "フェーズ1（減速）期間",     7.28, 4.22, 2.35, C.yellow);

  // トレードオフ
  addCard(s, 0.3, 4.4, 4.3, 0.88, "1A1A2E");
  addSectionLabel(s, "トレードオフ", 0.42, 4.44, C.muted);
  s.addText("プレイヤー弾は敵弾より意図的に「当たりやすく」設計（非対称バランス）。リアリズムより\nゲームフィールを優先。難易度はパラメータ化してあるため将来の調整が容易。", {
    x: 0.42, y: 4.65, w: 4.06, h: 0.55,
    fontFace: "Calibri", fontSize: 9, color: C.muted,
    align: "left", margin: 0,
  });
}

// ── スライド 4: ジャストエヴェイジョン ────────────────────────────────────
{
  const s = pres.addSlide();
  s.background = { color: C.bg };
  addSlideHeader(s, "ジャストエヴェイジョン — 入力ウィンドウ設計", "PlayerEvasion.cpp");

  // 上段：要件
  addCard(s, 0.3, 1.15, 9.4, 0.9);
  addSectionLabel(s, "実現したかったこと", 0.42, 1.20, C.blue);
  s.addText(
    "格闘ゲームのジャストガードと同様の「ギリギリ感」を出しつつ、カジュアルユーザーにも成功体験を与えたい。\n" +
    "かつ、フレームレートが変動しても判定がブレない実装が必要だった。",
    {
      x: 0.42, y: 1.44, w: 9.06, h: 0.54,
      fontFace: "Calibri", fontSize: 10.5, color: C.text,
      align: "left", margin: 0,
    }
  );

  // 比較テーブル
  addCard(s, 0.3, 2.18, 4.2, 1.5);
  addSectionLabel(s, "比較検討", 0.42, 2.23, C.yellow);
  s.addTable([
    ["案", "方式", "問題点"],
    ["A", "弾との距離判定",   "弾速変化のたびに閾値再調整が必要"],
    ["B", "フレーム数カウント","60fps固定前提、非依存にならない"],
    ["C★","秒数ウィンドウ",   "フレームレート非依存・調整しやすい"],
  ].map((r, i) => r.map((cell, ci) => ({
    text: cell,
    options: {
      fill: { color: i === 3 ? "1A3A2A" : (i === 0 ? C.panel2 : C.panel) },
      color: i === 3 ? C.green : (i === 0 ? C.muted : C.text),
      bold: i === 0 || i === 3,
      fontFace: ci === 0 ? "Consolas" : "Calibri",
      fontSize: i === 0 ? 8 : 9.5,
      align: ci === 0 ? "center" : "left",
    },
  }))), {
    x: 0.42, y: 2.48, w: 3.96, h: 1.1,
    colW: [0.35, 1.4, 2.21],
    border: { pt: 0.5, color: "374151" },
  });

  // コード
  addSectionLabel(s, "実装（実コード抜粋）", 4.7, 2.23, C.accent);
  addCode(s,
    `// PlayerEvasion.cpp\nbool PlayerEvasion::CheckJustEvasion() {\n    if (isDodging_\n        && justEvasionTimer_ <= justEvasionWindow_\n        && !hasJustEvaded_)\n    {\n        hasJustEvaded_ = true;  // 1回限り発火を保証\n        return true;            // ジャスト成立!\n    }\n    return false;\n}`,
    4.7, 2.48, 5.0, 2.0
  );

  // 数値
  addStat(s, "3.0s", "ジャスト受付ウィンドウ",  0.30, 3.82, 2.3, C.accent);
  addStat(s, "0.2s", "回避継続時間",              2.72, 3.82, 2.3, C.blue);
  addStat(s, "1.0s", "回避クールダウン",          5.14, 3.82, 2.3, C.green);
  addStat(s, "1回",  "発火の保証（フラグ管理）",  7.56, 3.82, 1.74, C.yellow);

  // ポイント
  addCard(s, 0.3, 4.9, 9.4, 0.5, "1A1A2E");
  s.addText(
    "「何フレーム」でなく「何秒」で管理することで、デザイナーが直感的にパラメータを触れる設計にした。" +
    "　トレードオフ：ウィンドウを広め（3.0s）に取ることで初心者にも成功体験を与えやすいが、上級者には物足りなく感じる可能性あり。",
    {
      x: 0.42, y: 4.97, w: 9.0, h: 0.38,
      fontFace: "Calibri", fontSize: 9.5, color: C.muted,
      align: "left", margin: 0,
    }
  );
}

// ── スライド 5: SRV 線形アロケーション ────────────────────────────────────
{
  const s = pres.addSlide();
  s.background = { color: C.bg };
  addSlideHeader(s, "DirectX 12 SRV デスクリプタヒープ — 線形アロケーション", "SrvManager.cpp");

  // 要件
  addCard(s, 0.3, 1.15, 9.4, 0.75);
  addSectionLabel(s, "実現したかったこと", 0.42, 1.20, C.blue);
  s.addText(
    "DirectX 12 では SRV（テクスチャ・構造化バッファ等）を自前のデスクリプタヒープで管理する必要がある。" +
    "このゲームは起動時に全アセットをロードし、ゲーム中の動的ロード/アンロードは行わない。",
    {
      x: 0.42, y: 1.44, w: 9.06, h: 0.40,
      fontFace: "Calibri", fontSize: 10.5, color: C.text,
      align: "left", margin: 0,
    }
  );

  // 比較（3案）
  const cases = [
    { title: "A. フリーリスト",        pros: "動的ロードに対応",               cons: "実装複雑・断片化リスク・このゲームには過剰", color: C.red    },
    { title: "B. プールアロケーター",   pros: "断片化を抑制",                   cons: "実装コスト同様に高い",                         color: C.yellow },
    { title: "C★ 線形アロケーション",  pros: "O(1)確保・実装シンプル",        cons: "解放不可（起動時ロードのみなら問題なし）",   color: C.green  },
  ];
  cases.forEach((c, i) => {
    addCard(s, 0.3 + i * 3.23, 2.05, 3.08, 1.72, c.title.startsWith("C") ? "0F2A1A" : C.panel);
    s.addShape(pres.shapes.RECTANGLE, {
      x: 0.3 + i * 3.23, y: 2.05, w: 3.08, h: 0.06,
      fill: { color: c.color }, line: { color: c.color, width: 0 },
    });
    s.addText(c.title, {
      x: 0.42 + i * 3.23, y: 2.15, w: 2.84, h: 0.35,
      fontFace: "Calibri", fontSize: 11, color: c.color,
      bold: true, align: "left", margin: 0,
    });
    s.addText("◎ " + c.pros, {
      x: 0.42 + i * 3.23, y: 2.54, w: 2.84, h: 0.38,
      fontFace: "Calibri", fontSize: 9.5, color: C.green,
      align: "left", margin: 0,
    });
    s.addText("✕ " + c.cons, {
      x: 0.42 + i * 3.23, y: 2.96, w: 2.84, h: 0.65,
      fontFace: "Calibri", fontSize: 9.5, color: C.muted,
      align: "left", margin: 0,
    });
  });

  // コード
  addSectionLabel(s, "実装（実コード抜粋）", 0.3, 3.88, C.accent);
  addCode(s,
    `// SrvManager.cpp\nuint32_t SrvManager::Allocate() {\n    uint32_t index = useIndex;\n    useIndex++;        // インクリメントのみ → O(1)\n    return index;\n    // 解放処理なし（起動時ロード専用のため不要）\n}`,
    0.3, 4.12, 4.5, 1.2
  );

  // 数値 + ポイント
  addStat(s, "1024",  "最大 SRV スロット数",  4.92, 4.12, 2.4, C.accent);
  addStat(s, "O(1)",  "確保コスト",            7.46, 4.12, 2.24, C.green);

  addCard(s, 0.3, 5.38, 9.4, 0.14, "1A1A2E");
}

// ── スライド 6: ビヘイビアツリー + パーティクル（2 in 1） ─────────────────
{
  const s = pres.addSlide();
  s.background = { color: C.bg };
  addSlideHeader(s, "ビヘイビアツリー × パーティクルファクトリ", "BehaviorTree.h  /  ParticleManager.cpp");

  // 左：BT
  addCard(s, 0.3, 1.15, 4.55, 4.25);
  addSectionLabel(s, "ビヘイビアツリー（自作）", 0.42, 1.20, C.blue);
  s.addText("要件：複数敵が共通の行動パターンを持ちつつ、種類ごとに攻撃内容を変えたい。\nFSM は状態数が増えると遷移条件が爆発するため、BT を自作した。", {
    x: 0.42, y: 1.44, w: 4.3, h: 0.65,
    fontFace: "Calibri", fontSize: 10, color: C.text,
    align: "left", margin: 0,
  });

  const btNodes = [
    { name: "Sequence",  desc: "全子が成功なら Success（AND）",  color: C.blue   },
    { name: "Selector",  desc: "最初の成功で打ち切り（OR）",     color: C.yellow },
    { name: "Condition", desc: "boolラムダ → 即時評価",          color: C.green  },
    { name: "Action",    desc: "処理ラムダ、Running状態で複数フレーム対応", color: C.accent },
  ];
  btNodes.forEach((n, i) => {
    s.addShape(pres.shapes.RECTANGLE, {
      x: 0.42, y: 2.18 + i * 0.7, w: 0.06, h: 0.42,
      fill: { color: n.color }, line: { color: n.color, width: 0 },
    });
    s.addText(n.name, {
      x: 0.56, y: 2.18 + i * 0.7, w: 1.2, h: 0.42,
      fontFace: "Consolas", fontSize: 10, color: n.color,
      align: "left", valign: "middle", margin: 0,
    });
    s.addText(n.desc, {
      x: 1.84, y: 2.18 + i * 0.7, w: 2.88, h: 0.42,
      fontFace: "Calibri", fontSize: 9.5, color: C.muted,
      align: "left", valign: "middle", margin: 0,
    });
  });

  // 数値（BT）
  addStat(s, "90°",  "敵の視野角",           0.42, 5.0 - 0.72, 1.35, C.blue);
  addStat(s, "50u",  "視野距離",              1.85, 5.0 - 0.72, 1.35, C.yellow);
  addStat(s, "0.5u", "レイサンプリング間隔",  3.28, 5.0 - 0.72, 1.38, C.accent);

  // 右：パーティクル
  addCard(s, 5.1, 1.15, 4.55, 4.25);
  addSectionLabel(s, "パーティクルファクトリレジストリ", 5.22, 1.20, C.accent);
  s.addText("要件：エフェクト種類を増やすたびに ParticleManager を修正したくない。\n文字列キーによるファクトリ登録で Open/Closed の原則を満たした。", {
    x: 5.22, y: 1.44, w: 4.3, h: 0.65,
    fontFace: "Calibri", fontSize: 10, color: C.text,
    align: "left", margin: 0,
  });
  addCode(s,
    `// 登録（一度だけ）\nemitterFactories_["Ring"] =\n    []() { return std::make_unique<RingEmitter>(); };\n\n// 生成（文字列キーのみでOK）\nauto emitter = emitterFactories_[typeName]();`,
    5.22, 2.18, 4.3, 1.38
  );

  const emitters = [
    "RingEmitter    — 回避ダッシュ",
    "OrbitTrailEmitter — 弾丸軌跡",
    "AuraEmitter    — ボスオーラ",
    "ExplosionEmitter — 爆発",
    "FireworkEmitter — ボス撃破",
  ];
  emitters.forEach((e, i) => {
    s.addText("→ " + e, {
      x: 5.22, y: 3.65 + i * 0.3, w: 4.3, h: 0.28,
      fontFace: "Consolas", fontSize: 9, color: C.muted,
      align: "left", margin: 0,
    });
  });

  addStat(s, "500k",   "最大インスタンス数",  5.22, 5.0 - 0.72, 2.0, C.accent);
  addStat(s, "50",     "アンドゥ履歴深度",    7.34, 5.0 - 0.72, 2.18, C.green);
}

// ── スライド 7: シーン遷移アニメーション ────────────────────────────────
{
  const s = pres.addSlide();
  s.background = { color: C.bg };
  addSlideHeader(s, "シーン遷移アニメーション — アルゴリズムによる有機的な演出", "SceneChangeAnimation.cpp");

  // 要件
  addCard(s, 0.3, 1.15, 9.4, 0.72);
  addSectionLabel(s, "実現したかったこと", 0.42, 1.20, C.blue);
  s.addText(
    "ブロックが崩れていくような遷移演出を実装したい。全ブロック一斉だと機械的、完全ランダムだと「波」の流れが失われる。",
    { x: 0.42, y: 1.44, w: 9.06, h: 0.36, fontFace: "Calibri", fontSize: 10.5, color: C.text, align: "left", margin: 0 }
  );

  // 選定理由 + コード
  addSectionLabel(s, "設計のポイント", 0.3, 2.0, C.accent);
  s.addText(
    "「左上→右下への対角線方向の波」を基本としつつ、乱数でバラつきを加える。\n" +
    "さらに後半ブロックでは乱数の影響を小さくして「波のまとまり」が崩れないようにした。",
    { x: 0.3, y: 2.24, w: 4.7, h: 0.72, fontFace: "Calibri", fontSize: 10.5, color: C.text, align: "left", margin: 0 }
  );
  addCode(s,
    `// SceneChangeAnimation.cpp\nfloat progress = (x + y) / maxIndexSum;\n//  ↑ 左上=0、右下=1 の対角線進行\n\nfloat randomOffset =\n    (rng % 1000) / 1000.0f * kDelayRandomJitterMax;\n\n// 後半ほどジッタの影響を小さくする重み付き\nblock.delay = duration * kDelayDurationRatio\n    * (progress + randomOffset * (1.0f - progress));`,
    0.3, 3.06, 4.7, 1.92
  );

  // 右：パラメータ可視化
  addCard(s, 5.2, 2.0, 4.5, 2.98);
  addSectionLabel(s, "定量データ", 5.32, 2.05, C.green);

  const params = [
    { label: "フェード割合",      val: "30 %", note: "全体時間の30%でフェード完了", color: C.accent },
    { label: "ディレイ展開割合",  val: "70 %", note: "70%の時間でブロックが時差展開", color: C.blue  },
    { label: "最大ランダムジッタ", val: "0.15s", note: "後半ブロックで自動縮小",    color: C.yellow },
  ];
  params.forEach((p, i) => {
    s.addShape(pres.shapes.RECTANGLE, {
      x: 5.32, y: 2.32 + i * 0.82, w: 4.24, h: 0.66,
      fill: { color: C.panel2 }, line: { color: p.color, width: 1.5 },
      shadow: makeShadow(),
    });
    s.addText(p.val, {
      x: 5.42, y: 2.38 + i * 0.82, w: 1.2, h: 0.35,
      fontFace: "Arial Black", fontSize: 18, color: p.color,
      align: "center", margin: 0,
    });
    s.addText(p.label, {
      x: 6.72, y: 2.38 + i * 0.82, w: 2.72, h: 0.3,
      fontFace: "Calibri", fontSize: 10.5, color: C.text,
      bold: true, align: "left", margin: 0,
    });
    s.addText(p.note, {
      x: 6.72, y: 2.68 + i * 0.82, w: 2.72, h: 0.24,
      fontFace: "Calibri", fontSize: 8.5, color: C.muted,
      align: "left", margin: 0,
    });
  });

  // トレードオフ
  addCard(s, 0.3, 5.07, 9.4, 0.5, "1A1A2E");
  s.addText(
    "トレードオフ：アルゴリズム生成のため、手付けキーフレームに比べ特定タイミングへの細かい演出制御は難しい。" +
    "　しかしパラメータ（比率・ジッタ量）を変えるだけで雰囲気を調整でき、反復改善には向いている。",
    { x: 0.42, y: 5.14, w: 9.06, h: 0.36, fontFace: "Calibri", fontSize: 9.5, color: C.muted, align: "left", margin: 0 }
  );
}

// ── スライド 8: まとめ ────────────────────────────────────────────────────
{
  const s = pres.addSlide();
  s.background = { color: C.bg };

  s.addShape(pres.shapes.RECTANGLE, {
    x: 0, y: 0, w: 10, h: 1.1,
    fill: { color: "0D1117" }, line: { color: "0D1117", width: 0 },
  });
  s.addShape(pres.shapes.RECTANGLE, {
    x: 0, y: 1.1, w: 10, h: 0.04,
    fill: { color: C.accent }, line: { color: C.accent, width: 0 },
  });
  s.addText("まとめ — 技術選定で一貫した判断軸", {
    x: 0.4, y: 0.12, w: 9, h: 0.65,
    fontFace: "Arial Black", fontSize: 20, color: C.text,
    align: "left", bold: true, margin: 0,
  });
  s.addText("各技術は「何を使ったか」より「なぜそれを選んだか」を軸に説明した", {
    x: 0.4, y: 0.72, w: 9, h: 0.3,
    fontFace: "Calibri", fontSize: 11, color: C.muted,
    align: "left", margin: 0,
  });

  const summaries = [
    { title: "ホーミング弾",          key: "ターンアシスト",      value: "視覚×確実性の両立", color: C.accent },
    { title: "ジャストエヴェイジョン", key: "秒数ウィンドウ",      value: "fps非依存・調整容易", color: C.blue   },
    { title: "SRV 線形アロケーション", key: "O(1)確保",            value: "要件に十分な最小解", color: C.green  },
    { title: "ビヘイビアツリー自作",   key: "関数合成180行",       value: "ライブラリ不要の規模", color: C.yellow },
    { title: "パーティクルファクトリ", key: "文字列キー登録",      value: "開放/閉鎖の原則", color: "A78BFA" },
    { title: "シーン遷移アニメ",       key: "対角線ディレイ",      value: "アルゴリズムで有機的", color: "F472B6" },
  ];

  summaries.forEach((item, i) => {
    const col = i % 2;
    const row = Math.floor(i / 2);
    const x = 0.3 + col * 4.9;
    const y = 1.25 + row * 1.3;

    addCard(s, x, y, 4.6, 1.1);
    s.addShape(pres.shapes.RECTANGLE, {
      x, y, w: 4.6, h: 0.05,
      fill: { color: item.color }, line: { color: item.color, width: 0 },
    });
    s.addText(item.title, {
      x: x + 0.15, y: y + 0.12, w: 4.3, h: 0.3,
      fontFace: "Calibri", fontSize: 12, color: C.text,
      bold: true, align: "left", margin: 0,
    });
    s.addText(item.key, {
      x: x + 0.15, y: y + 0.46, w: 2.0, h: 0.26,
      fontFace: "Consolas", fontSize: 9.5, color: item.color,
      align: "left", margin: 0,
    });
    s.addText("→ " + item.value, {
      x: x + 2.2, y: y + 0.46, w: 2.25, h: 0.26,
      fontFace: "Calibri", fontSize: 9.5, color: C.muted,
      align: "left", margin: 0,
    });
    s.addText("要件に対して十分な解を選ぶ / 汎用性よりシンプルさ / 定量的に検証する", {
      x: x + 0.15, y: y + 0.76, w: 4.3, h: 0.22,
      fontFace: "Calibri", fontSize: 8, color: "4B5563",
      align: "left", margin: 0,
    });
  });

  s.addText("Tsuboki Hayato  /  TuboEngine  /  2026", {
    x: 0, y: 5.3, w: 10, h: 0.3,
    fontFace: "Calibri", fontSize: 9, color: "374151",
    align: "center", margin: 0,
  });
}

// ── 出力 ────────────────────────────────────────────────────────────────────
const outPath = "C:\\Users\\tubosan\\source\\repos\\TuboEngine\\TuboEngine_技術説明スライド.pptx";
pres.writeFile({ fileName: outPath })
    .then(() => console.log("出力完了:", outPath))
    .catch(e  => { console.error(e); process.exit(1); });
