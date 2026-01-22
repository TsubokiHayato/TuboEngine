param(
  # ---------------------------------------------------------------------------
  # 基本パラメータ
  # ---------------------------------------------------------------------------
  # `ProjectDir` : `.sln` / `.vcxproj` / `.vcxproj.filters` が存在するディレクトリ
  #   - 既定ではこのスクリプト(`tools\RegenerateFilters.ps1`)の1つ上(`project\`)を指す
  [string]$ProjectDir = (Resolve-Path "$PSScriptRoot\.."),

  # `VcxprojFilters` : 再生成対象の `.vcxproj.filters` ファイル名
  [string]$VcxprojFilters = "CG2_01.vcxproj.filters",

  # `Vcxproj` : 参照元の `.vcxproj` ファイル名
  #   - プロジェクトに「実際に含まれている(= Includeされている)ファイル」だけを `.filters` 側に載せるために読む
  [string]$Vcxproj = "CG2_01.vcxproj",

  # `RootFolders` : Filterの元にするルートフォルダ
  #   - 例: `engine\graphic\...` のような階層を、そのまま Visual Studio の仮想フォルダ(Filter)にする
  #   - 必要なら `resources` / `shaders` なども追加してOK
  [string[]]$RootFolders = @("engine","application"),

  # `Extensions` : 対象にする拡張子
  #   - ここに無い拡張子のファイルはスキャン対象外
  #   - `.vcxproj` に含まれていても、拡張子が対象外なら `.filters` には出ない
  [string[]]$Extensions = @(".h",".hpp",".inl",".cpp",".c",".txt",".md",".hlsl",".rc")
)

# -----------------------------------------------------------------------------
# 実行時の堅牢化設定
# -----------------------------------------------------------------------------
# 変数の未定義参照などをエラーにし、バグを早期に検出
Set-StrictMode -Version Latest

# 途中で問題が起きたら即停止する(黙って壊れた `.filters` を出さない)
$ErrorActionPreference = "Stop"

# -----------------------------------------------------------------------------
# 入力パスの解決 / 存在チェック
# -----------------------------------------------------------------------------
$projDir = Resolve-Path $ProjectDir
$filtersPath = Join-Path $projDir $VcxprojFilters
$vcxprojPath = Join-Path $projDir $Vcxproj

if (!(Test-Path $filtersPath)) { throw "Not found: $filtersPath" }
if (!(Test-Path $vcxprojPath)) { throw "Not found: $vcxprojPath" }

# -----------------------------------------------------------------------------
# ユーティリティ: 絶対パス -> ベースからの相対パス(Windows区切り)へ変換
# -----------------------------------------------------------------------------
# `.vcxproj` の Include は通常 "engine\\...\\file.cpp" のような相対パスで記述されるため、
# こちらも同じ表記に揃えて HashSet で突合できるようにする。
function Get-RelPath([string]$base, [string]$full) {
  $baseUri = (New-Object System.Uri(($base.TrimEnd('\\') + '\\')))
  $fullUri = New-Object System.Uri($full)
  return [System.Uri]::UnescapeDataString($baseUri.MakeRelativeUri($fullUri).ToString().Replace('/','\\'))
}

# -----------------------------------------------------------------------------
# 1) 指定ルートフォルダ配下を走査して「候補ファイル一覧」を作る
# -----------------------------------------------------------------------------
# ここでは「ディスク上に存在する」ファイルを拾う。
# その後 `.vcxproj` を読んで「プロジェクトに含まれているか」を判定して絞り込む。
$allFiles = @()
foreach ($root in $RootFolders) {
  $rootPath = Join-Path $projDir $root
  if (Test-Path $rootPath) {
    $allFiles += Get-ChildItem -Path $rootPath -Recurse -File |
      Where-Object { $Extensions -contains $_.Extension.ToLowerInvariant() }
  }
}

# 重複除去 + 安定ソート(毎回同じ順序で出力して差分が荒れないようにする)
$allFiles = $allFiles | Sort-Object FullName -Unique

# -----------------------------------------------------------------------------
# 2) `.vcxproj` を解析して「実際に Include されている」パスだけを収集
# -----------------------------------------------------------------------------
# `.filters` はIDE表示用なので、プロジェクトに入っていないファイルを載せると混乱する。
# そのため `.vcxproj` に記載のある項目のみ対象にする。
[xml]$projXml = Get-Content $vcxprojPath
$ns = New-Object System.Xml.XmlNamespaceManager($projXml.NameTable)
$ns.AddNamespace("msb", $projXml.Project.NamespaceURI)

# Include パスのセット(高速に存在判定するため HashSet を利用)
$included = New-Object System.Collections.Generic.HashSet[string]
foreach ($nodeName in @("ClCompile","ClInclude","None","ResourceCompile")) {
  $nodes = $projXml.SelectNodes("//msb:$nodeName", $ns)
  foreach ($n in $nodes) {
    $inc = $n.GetAttribute("Include")
    if (![string]::IsNullOrWhiteSpace($inc)) { [void]$included.Add($inc) }
  }
}

# -----------------------------------------------------------------------------
# 3) `Include -> Filter(=親フォルダ)` の対応表を作る
# -----------------------------------------------------------------------------
# Visual Studio の Filter は「仮想フォルダ名」。
# ここでは「ファイルの親ディレクトリ」をそのまま Filter 名として採用する。
$items = @()
foreach ($f in $allFiles) {
  $rel = Get-RelPath $projDir $f.FullName

  # `.vcxproj` に含まれていないものは対象外
  if (!$included.Contains($rel)) { continue }

  # 例: engine\graphic\3d\Object3dCommon.cpp -> Filter = engine\graphic\3d
  $filter = Split-Path $rel -Parent
  if ([string]::IsNullOrWhiteSpace($filter) -or $filter -eq ".") { $filter = "" }

  $items += [pscustomobject]@{ Include = $rel; Filter = $filter; Ext = $f.Extension.ToLowerInvariant() }
}

# -----------------------------------------------------------------------------
# 4) 既存 `.vcxproj.filters` を読み込み、ItemGroup を全消ししてから再構築
# -----------------------------------------------------------------------------
# ※VSが読むXMLの "Project" / 名前空間などの外枠は流用し、
#   中身(ItemGroup)だけを「決定的(Deterministic)」に作り直す。
[xml]$filtersXml = Get-Content $filtersPath
$ns2 = New-Object System.Xml.XmlNamespaceManager($filtersXml.NameTable)
$ns2.AddNamespace("msb", $filtersXml.Project.NamespaceURI)

if ($filtersXml.Project -eq $null) { throw "Invalid .filters" }

# 既存の ItemGroup を除去
# `Project.ItemGroup` は PowerShell の列挙タイミングによって
# 文字列扱いになったり安全に削除できないケースがあるため、
# XML API でノード一覧を取得してから削除する。
$itemGroups = @($filtersXml.SelectNodes("//msb:Project/msb:ItemGroup", $ns2))
foreach ($ig in $itemGroups) {
  [void]$filtersXml.Project.RemoveChild($ig)
}

# -----------------------------------------------------------------------------
# 5) Filter 定義(仮想フォルダ一覧)を生成
# -----------------------------------------------------------------------------
# `.filters` の先頭側に `<Filter Include="...">` を列挙しておくと、
# VS上でフォルダとして表示される。
$filterSet = New-Object System.Collections.Generic.SortedSet[string]
foreach ($it in $items) {
  if (![string]::IsNullOrWhiteSpace($it.Filter)) { [void]$filterSet.Add($it.Filter) }
}

$igFilters = $filtersXml.CreateElement("ItemGroup", $filtersXml.Project.NamespaceURI)
foreach ($flt in $filterSet) {
  $fnode = $filtersXml.CreateElement("Filter", $filtersXml.Project.NamespaceURI)
  [void]$fnode.SetAttribute("Include", $flt)

  # UniqueIdentifier は VS が内部的に参照するため慣例的に付与する
  $uid = $filtersXml.CreateElement("UniqueIdentifier", $filtersXml.Project.NamespaceURI)
  $uid.InnerText = "{" + [guid]::NewGuid().ToString().ToUpperInvariant() + "}"
  [void]$fnode.AppendChild($uid)

  [void]$igFilters.AppendChild($fnode)
}
[void]$filtersXml.Project.AppendChild($igFilters)

# -----------------------------------------------------------------------------
# 6) 各ファイル項目を種類別に ItemGroup へ追加
# -----------------------------------------------------------------------------
# `.filters` では、ファイルの種類に応じてノード名が異なる。
# 例:
#   - `.cpp` -> `<ClCompile Include="...">`
#   - `.h`   -> `<ClInclude Include="...">`
#   - `.rc`  -> `<ResourceCompile Include="...">`
#   - その他 -> `<None Include="...">`
function Add-ItemsGroup([string]$itemName, [object[]]$arr) {
  if ($arr.Count -eq 0) { return }

  $ig = $filtersXml.CreateElement("ItemGroup", $filtersXml.Project.NamespaceURI)
  foreach ($it in $arr) {
    $node = $filtersXml.CreateElement($itemName, $filtersXml.Project.NamespaceURI)
    [void]$node.SetAttribute("Include", $it.Include)

    # Filter が空でなければ割り当てる(空なら (none) 扱い)
    if (![string]::IsNullOrWhiteSpace($it.Filter)) {
      $f = $filtersXml.CreateElement("Filter", $filtersXml.Project.NamespaceURI)
      $f.InnerText = $it.Filter
      [void]$node.AppendChild($f)
    }

    [void]$ig.AppendChild($node)
  }
  [void]$filtersXml.Project.AppendChild($ig)
}

# 種類ごとに振り分け
$clCompile = @(); $clInclude = @(); $none = @(); $rc = @()
foreach ($it in $items) {
  switch ($it.Ext) {
    ".cpp" { $clCompile += $it }
    ".c"   { $clCompile += $it }
    ".h"   { $clInclude += $it }
    ".hpp" { $clInclude += $it }
    ".inl" { $clInclude += $it }
    ".rc"  { $rc += $it }
    default { $none += $it }
  }
}

Add-ItemsGroup "ClCompile" $clCompile
Add-ItemsGroup "ClInclude" $clInclude
Add-ItemsGroup "None" $none
Add-ItemsGroup "ResourceCompile" $rc

# -----------------------------------------------------------------------------
# 7) 保存(UTF-8 BOM + 改行CRLF)
# -----------------------------------------------------------------------------
# Visual Studio / MSBuild XML は UTF-8 BOM 付きでも問題なく、
# 文字化け回避に寄与するため BOM を付けて保存する。
$utf8Bom = New-Object System.Text.UTF8Encoding($true)
$writerSettings = New-Object System.Xml.XmlWriterSettings
$writerSettings.Indent = $true
$writerSettings.Encoding = $utf8Bom
$writerSettings.NewLineChars = "`r`n"
$writerSettings.NewLineHandling = "Replace"

$writer = [System.Xml.XmlWriter]::Create($filtersPath, $writerSettings)
$filtersXml.Save($writer)
$writer.Close()

Write-Host "Regenerated: $filtersPath"
