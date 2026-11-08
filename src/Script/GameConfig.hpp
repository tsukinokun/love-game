//--------------------------------------------------------------
//! @file   GameConfig.hpp
//! @brief  Assets/Data の設定ファイル。ゲーム本体と検証ツールで共有する
//--------------------------------------------------------------
#pragma once
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace LoveGame::Script {
    //! @brief 3次元の値
    struct Vec3 {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    //! @brief game.json: ゲーム進行の値
    struct GameConfig {
        int   statMax       = 0;       // 能力値の上限
        int   maxNameLength = 0;       // 主人公の名前の文字数の上限
        int   slotCount     = 0;       // セーブスロットの数
        float fadeSeconds   = 0.0f;    // 暗転にかける秒数
    };

    //! @brief ui.json: UIの動きと、Prefabに書けない可変の文言
    struct UiConfig {
        float charsPerSecond   = 0.0f;    // 文字送りの速さ
        float fullWidthPerLine = 0.0f;    // 全角換算の1行の文字数
        std::string noLineStart;          // 行頭に置けない文字
        std::string noLineEnd;            // 行末に置けない文字

        float listCenterX   = 0.0f;    // 縦に並ぶボタンの中心X
        float listGap       = 0.0f;    // ボタンの間隔
        float actionCenterX = 0.0f;    // カレンダーの自分磨きの列の中心X
        float placeCenterX  = 0.0f;    // カレンダーの会いに行く列の中心X
        float choiceCenterY = 0.0f;    // 選択肢の並びの中心Y
        float slotCenterY   = 0.0f;    // セーブ枠の並びの中心Y
        float placeCenterY  = 0.0f;    // カレンダーの場所の並びの中心Y

        float statusY    = 0.0f;    // カレンダーの好感度の表示を始めるY
        float statusStep = 0.0f;    // 好感度の表示の行の間隔
        float statX      = 0.0f;    // 能力値の表示のX
        float affectionX = 0.0f;    // 好感度の表示のX

        float candidateX = 0.0f;    // 変換候補の窓のX
        float candidateY = 0.0f;    // 変換候補の窓のY

        std::string dateFormat;           // {month} {week}
        std::string weekFormat;           // {date} {current} {total}
        std::string statusFormat;         // {name} {affection}
        std::string statFormat;           // {name} {value}
        std::string effectFormat;         // {name} {value}
        std::string appointmentFormat;    // {date} {name}
        std::string resultHint;           // 自分磨きの結果の下に出す案内
        std::string slotFilled;           // {slot} {date} {name}
        std::string slotEmpty;            // {slot}
        std::string slotClose;            // セーブ画面を閉じるボタン
    };

    //! @brief カメラの位置と注視点
    struct CameraShot {
        Vec3 position;
        Vec3 target;
    };

    //! @brief stage.json: 3D舞台の演出の値
    struct StageConfig {
        std::string defaultStage;     // 台本が指定する前の舞台
        std::string defaultCamera;    // 台本が指定する前のカメラ
        float       cameraSpeed = 0.0f;

        std::map<std::string, std::string> stages;         // 舞台名 → 床と壁のマテリアル
        std::map<std::string, float>       positions;      // 立ち位置 → X座標
        std::map<std::string, CameraShot>  cameraShots;    // カメラ名 → 位置と注視点
    };

    //! @brief モデル確認シーンの1場面
    struct PreviewShot {
        std::string name;        // 画面に出す名前
        std::string group;       // 表示する *.Group.json
        Vec3        position;    // カメラの位置
        Vec3        target;      // カメラの注視点
    };

    //! @brief preview.json: モデル確認シーンの場面
    struct PreviewConfig {
        std::vector<PreviewShot> shots;
    };

    //! @brief calendar.json の1か月
    struct MonthDef {
        std::string name;         // 画面に出す月の名前
        int         weeks = 0;    // その月の週数
    };

    //! @brief calendar.json の能力値
    struct StatDef {
        std::string id;      // 変数名 stat.<id> に使う識別子
        std::string name;    // 画面に出す名前
    };

    //! @brief calendar.json の自分磨きの行動
    struct ActionDef {
        std::string                label;      // ボタンの文字
        std::string                result;     // 実行したときの文
        std::map<std::string, int> effects;    // 能力値の id → 増える量
    };

    //! @brief calendar.json: 暦と自分磨き
    struct CalendarConfig {
        std::vector<MonthDef>  months;
        std::vector<StatDef>   stats;
        std::vector<ActionDef> actions;

        //! @brief 全部で何週あるか
        int TotalWeeks() const;

        //! @brief id の能力値を探す。無ければ nullptr
        const StatDef* FindStat(const std::string& id) const;
    };

    //! @brief calendar.json を読む。失敗したら false
    bool LoadCalendarConfig(const std::filesystem::path& path, CalendarConfig& out, std::string& error);

    //! @brief preview.json を読む。失敗したら false
    bool LoadPreviewConfig(const std::filesystem::path& path, PreviewConfig& out, std::string& error);

    //! @brief game.json を読む。失敗したら false
    bool LoadGameConfig(const std::filesystem::path& path, GameConfig& out, std::string& error);

    //! @brief ui.json を読む。失敗したら false
    bool LoadUiConfig(const std::filesystem::path& path, UiConfig& out, std::string& error);

    //! @brief stage.json を読む。失敗したら false
    bool LoadStageConfig(const std::filesystem::path& path, StageConfig& out, std::string& error);
}    // namespace LoveGame::Script
