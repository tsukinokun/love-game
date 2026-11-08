//--------------------------------------------------------------
//! @file   Game.hpp
//! @brief  シーンをまたいで生きるゲーム全体の状態
//--------------------------------------------------------------
#pragma once
#include <Script/CharacterTable.hpp>
#include <Script/GameConfig.hpp>
#include <Script/Program.hpp>
#include <Script/Runner.hpp>

#include <map>
#include <string>
#include <vector>

namespace LoveGame {
    //! @brief シーン名
    namespace SceneName {
        inline constexpr const char* kTitle     = "Title";
        inline constexpr const char* kNameEntry = "NameEntry";
        inline constexpr const char* kCalendar  = "Calendar";
        inline constexpr const char* kStage     = "Stage";
        inline constexpr const char* kPreview   = "Preview";
    }    // namespace SceneName

    //! @brief カレンダーに入った約束
    struct Appointment {
        int         week = 0;      // 約束の週
        std::string scriptFile;    // デートの台本ファイル
        std::string label;         // デートの開始ラベル
    };

    //! @brief セーブの対象になる状態
    struct GameState {
        std::string       playerName;    // 主人公の名前（UTF-8）
        int               week = 1;      // 今が何週目か（1始まり）
        Script::Variables variables;     // 台本の変数（好感度、進行度、能力値）

        std::vector<Appointment> appointments;    // 未消化の約束。週の早い順

        bool        inConversation = false;    // 会話の途中か
        bool        isEnding       = false;    // 今の会話がエンディングか
        std::string scriptFile;                // 会話中の台本ファイル名
        std::string startLabel;                // 会話の開始ラベル
        std::string lineId;                    // 再開する行ID。空なら startLabel から始める

        std::string                        stageName;      // 表示中の舞台
        std::string                        cameraShot;     // 表示中のカメラ
        std::map<std::string, std::string> shownActors;    // 表示中のキャラ → 立ち位置

        //! @brief 変数の値を読む。未設定なら 0
        int Get(const std::string& name) const;
    };

    //! @brief ゲーム全体で共有するもの。WinMain が所有し、シーンへ参照を渡す
    struct Game {
        GameState              state;
        Script::CharacterTable characters;
        Script::GameConfig     config;
        Script::UiConfig       ui;
        Script::StageConfig    stage;
        Script::CalendarConfig calendar;
        Script::PreviewConfig  preview;
        int                    previewIndex = 0;    // モデル確認シーンで表示中の場面

        //! @brief Assets/Data の設定をすべて読む
        //! @param error [out] 失敗の内容
        //! @return すべて読めたら true
        bool LoadData(std::string& error);

        //! @brief 台本ファイルを読み、問題があればログへ出す
        //! @param file    [in]  Assets/Scripts からの相対パス
        //! @param program [out] 解析結果
        //! @return 読めて、解析エラーが無ければ true
        bool LoadProgram(const std::string& file, Script::Program& program) const;

        //! @brief 新しいゲームを始める状態にする
        void ResetState();

        //! @brief 会話を始める状態にする。舞台の状態は既定に戻す
        //! @param script   [in] 台本ファイル名
        //! @param label    [in] 開始ラベル
        //! @param isEnding [in] エンディングか
        void BeginConversation(const std::string& script, const std::string& label, bool isEnding);

        //! @brief 週を進める。予定があれば会話を始める状態にする
        //! @return 次のシーン名
        const char* FinishWeek();

        //! @brief 約束を追加する。その週が埋まっていれば次の空いている週へずらす
        //! @param script     [in] デートの台本ファイル名
        //! @param label      [in] デートの開始ラベル
        //! @param weeksLater [in] 何週後か
        //! @return 最終週より後になるときは追加せず false
        bool AddAppointment(const std::string& script, const std::string& label, int weeksLater);

        //! @brief 週番号を「4月 第1週」のような文字列にする
        std::string FormatDate(int week) const;
    };
}    // namespace LoveGame
