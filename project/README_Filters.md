# Visual Studio の `(none)` を直す（`.vcxproj.filters` 再生成手順）

このリポジトリでは、Visual Studio のソリューション エクスプローラー上の
**仮想フォルダ（Filter）** を `.vcxproj.filters` で管理しています。

`.vcxproj.filters` が未整備だと、ファイルがすべて **`(none)`** 配下に表示されます。
その状態を直すために、`tools\RegenerateFilters.ps1` で `.filters` を再生成します。

---

## いつ実行する？

次のような作業をした後に実行してください。

- 新しい `.cpp` / `.h` などを追加した
- ファイルやフォルダを移動した
- ファイルを削除した

※ **F5（デバッグ実行）では更新されません。**

---

## 実行方法（初心者向け）

作業ディレクトリは、`CG2_01.sln` がある `project\` フォルダです。

例：`C:\Users\tubosan\source\repos\TuboEngine\project\`

### 方法A：PowerShell で 1回だけ実行（実行制限に強い・推奨）

1. `project\` フォルダで PowerShell を開く
2. 次をそのまま入力して Enter

- `powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\RegenerateFilters.ps1`

成功すると、次のような表示が出ます：

- `Regenerated: ...CG2_01.vcxproj.filters`


### 方法B：実行ポリシーを設定して、以後は短いコマンドで実行

> 何度も使う人向け。管理者権限は不要です（CurrentUserのみ変更）。

1. 現在の状態確認

- `Get-ExecutionPolicy -List`

2. CurrentUser だけ許可（おすすめ：RemoteSigned）

- `Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy RemoteSigned`

3. 以後は、`project\` でこれだけで実行OK

- `.\tools\RegenerateFilters.ps1`

---

## 実行後に Visual Studio に反映させる

`.filters` は IDE 表示用なので、VS がすぐ反映しないことがあります。

- まず：ソリューション エクスプローラーで **プロジェクトを右クリック →「再読み込み」**
- だめなら：**Visual Studio を閉じて開き直す**

---

## 何をしているスクリプト？（概要）

`tools\RegenerateFilters.ps1` は以下を行います。

- `engine\` と `application\` を走査して候補ファイルを集める
- `.vcxproj` を解析して **プロジェクトに含まれているファイル（Include）だけ** に絞る
- `.vcxproj.filters` を再構築して、実フォルダ構造と同じ Filter を割り当てる

---

## カスタマイズ

- `resources` や `shaders` も対象にしたい場合は、スクリプト先頭の `RootFolders` に追加してください。
- 対象拡張子を増やす場合は `Extensions` に追加してください。
