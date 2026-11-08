# LoveGame

TsukinoEngine で作る、女性同士の恋愛を描く3D恋愛ゲーム。カレンダー育成型で、会話は3D舞台で進む。
モデルが無いので、当面キャラは色違いのブロックで表す。応答は日本語。

## ビルドと実行

```
build.bat            Debug をビルド（premake の再生成も毎回行う）
build.bat Release
bin\Debug\LoveGame.exe              実行時の作業フォルダはリポジトリルート
bin\Debug\LoveGame.ScriptCheck.exe  台本とデータの検証
bin\Debug\LoveGame.ScriptCheck.exe --stamp   行IDの付与と検証
```

ログは `Logs/LoveGame.log`。エンジンの API は `External/TsukinoEngine/CLAUDE.md` の案内に従って調べる。
`External/TsukinoEngine/External/` は読まない。

## 構成

| 場所 | 役割 |
|---|---|
| `src/Script/` | 台本の解析と実行。エンジンに依存しない静的ライブラリ |
| `src/Game/` | シーンをまたぐ状態（`GameState` がセーブ対象） |
| `src/Save/` | セーブ。保存先は `%LOCALAPPDATA%\LoveGame\Saves\` |
| `src/UI/` | メッセージ窓、ボタン列、折り返し |
| `src/Scene/` | `GameScene` が全シーン共通の土台 |
| `Tools/ScriptCheck/` | 台本と Assets のデータの検証ツール |
| `Assets/Scripts/` | 台本（ヒロインごとに1ファイル） |
| `Assets/Data/` | システムの値 |
| `Assets/Prefabs/` | コンポーネントの値 |

## 値の置き場所

**調整できる値はC++に書かない。** コードに残すのはシステムの登録順、シーン名、Prefabのパスなどの識別子だけ。

| 値 | 置き場所 |
|---|---|
| 能力値の上限、スロット数、暗転時間 | `Assets/Data/game.json` |
| 暦(月と週数)、能力値の種類、自分磨きの行動 | `Assets/Data/calendar.json` |
| 文字送り、折り返し、ボタン列の配置、可変の文言 | `Assets/Data/ui.json` |
| 立ち位置、カメラショット、舞台のマテリアル | `Assets/Data/stage.json` |
| ヒロインの定義、イベントの条件(`eventRequires`) | `Assets/Data/characters.json` |
| UIと3D物体の配置、色、大きさ、固定の文言 | `Assets/Prefabs/<シーン>/` |

Prefabの決まりは次のとおり。

- シーンの全エンティティは `<シーン>/Scene.Group.json` に並べ、`GameScene::LoadLayout` で生成して `Find("名前")` で引く
- 1エンティティは `Prefab.json`(目次)とコンポーネントごとのJSONで構成する。共通の値は `Common/` のファイルを目次から参照する
- 値を持たないコンポーネントは目次の値を `"null"` にする
- 個数が変わるUIは `Common/Button` などの雛形から生成し、位置と文字だけコードで変える
- 文言は `UITextComponent` に書く。`FontComponent::text` はPrefabに保存されない
- **コンポーネントJSONの項目は順番を変えない。** エンジンの多くの `load` は項目を順番で読む。既存のファイルか `External/TsukinoEngine/Docs/components.md` の順に合わせる
- 値を変えたら `LoveGame.ScriptCheck` を実行する。壊れたJSONは実行時に例外になる

## 台本の書式

```
; コメント
*ラベル
@stage library          舞台（stage.json の stages）
@show akari center      立ち位置（stage.json の positions）
@hide akari
@camera close           カメラ（stage.json の cameraShots）
あかり「台詞」 #a1f3     話者は characters.json の name か {player}
地の文 #8697
@choice #eb4c
- 選択肢の文 -> 飛び先
  affection.akari += 5
@if affection.akari >= 10 -> ラベル
@set progress.akari = 1
@appoint date_1 2       2週後に date_1 の約束を入れる
@jump ラベル
@end
```

- 行ID（`#xxxx`）は `--stamp` で付ける。セーブ位置に使うので、文章を直しても変えないこと
- 毎週、自分磨きかヒロインに会いに行くかを1つ選ぶ。能力値は変数 `stat.<id>` に入るので、台本の `@if` でも読める
- 会いに行くと `event_<進行度+1>` を探し、無いか `eventRequires` を満たさなければ `daily` を再生する。全週の後は好感度が最も高い相手の `ending` を再生する
- `@appoint` の週が埋まっていれば次の空いている週へずれる。約束の週になると、選択を挟まずその会話が始まる
- `@bgm` と `@se` は文法だけ受け付け、音はまだ鳴らない

## 注意点

- `SkyAtmosphereSystem` は `StageScene` でだけ登録する。空の無いシーンで動かすと環境光が壊れ、以後の物体が真っ黒になる
- `.tmat` の色はカンマ区切り（`BaseColor = 1, 1, 1, 1`）
- `FontComponent` に `UIRectComponent` や `maxWidth` を付けると、長い文は折り返さず縮小される。本文は `TextLayout::Wrap` で事前に折り返す

## モデルの確認

`bin\Debug\LoveGame.exe preview 2` のように起動すると、`Assets/Data/preview.json` の場面を表示する。数字キーで切り替える。
KayKit のFBXは倍率1でcm単位に合う。追加したモデルの出どころは `Assets/Models/LICENSES.md` に記録する。
校舎のFBXは倍率4で実寸に合い、中心が原点なので `y = 672` に置く。再配布禁止のためGitに入れない。`LoveGame.ScriptCheck` はモデルファイルの有無を調べないので、`Assets/Models/School/School_building.fbx` が無くても検証は通る。
