//--------------------------------------------------------------
//! @file   Program.hpp
//! @brief  解析済みの台本データ
//--------------------------------------------------------------
#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace LoveGame::Script {
    //! @brief 変数への代入の種類
    enum class AssignOp { Set, Add, Subtract };

    //! @brief 比較の種類
    enum class CompareOp { Equal, NotEqual, Less, LessEqual, Greater, GreaterEqual };

    //! @brief 変数への代入（例: affection.akari += 5）
    struct Assignment {
        std::string variable;
        AssignOp    op    = AssignOp::Set;
        int         value = 0;
    };

    //! @brief 選択肢の1項目
    struct ChoiceOption {
        std::string             text;
        std::string             targetLabel;
        std::vector<Assignment> effects;
        int                     sourceLine = 0;
    };

    //! @brief 命令の種類
    enum class OpType {
        Text,       // 台詞または地の文
        Choice,     // 選択肢
        Jump,       // 無条件ジャンプ
        If,         // 条件ジャンプ
        Set,        // 変数の代入
        Command,    // 演出命令（ゲーム側が解釈する）
        End,        // 台本の終了
    };

    //! @brief 台本の1命令
    struct Op {
        OpType      type       = OpType::End;
        int         sourceLine = 0;    // 台本ファイルの行番号（1始まり）
        std::string lineId;            // 行ID（Text / Choice のみ）

        std::string speaker;    // Text: 話者。空なら地の文
        std::string text;       // Text: 本文

        std::vector<ChoiceOption> options;    // Choice

        std::string targetLabel;    // Jump / If

        std::string conditionVariable;                    // If
        CompareOp   conditionOp    = CompareOp::Equal;    // If
        int         conditionValue = 0;                   // If

        Assignment assignment;    // Set

        std::string              command;    // Command: 命令名
        std::vector<std::string> args;       // Command: 引数
    };

    //! @brief 台本1ファイル分のデータ
    struct Program {
        std::string                          name;
        std::vector<Op>                      ops;
        std::unordered_map<std::string, int> labels;     // ラベル名 → ops の添字
        std::unordered_map<std::string, int> lineIds;    // 行ID → ops の添字
    };

    //! @brief 解析と検証で見つかった問題
    struct Diagnostic {
        int         line = 0;
        std::string message;
    };
}    // namespace LoveGame::Script
