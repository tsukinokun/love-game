//--------------------------------------------------------------
//! @file   Validator.hpp
//! @brief  台本の意味の検証
//--------------------------------------------------------------
#pragma once
#include <Script/CharacterTable.hpp>
#include <Script/GameConfig.hpp>
#include <Script/Program.hpp>

#include <vector>

namespace LoveGame::Script {
    //! @brief 1以上の整数の文字列を読む。@appoint の週数に使う
    //! @param text  [in]  文字列
    //! @param value [out] 読めた値
    //! @return 全体が1以上の整数なら true
    bool ParsePositiveInt(const std::string& text, int& value);

    //! @brief 飛び先、話者、演出命令の引数、舞台やカメラの名前、行IDの漏れを検証する
    std::vector<Diagnostic> Validate(const Program& program, const CharacterTable& characters, const StageConfig& stage);
}    // namespace LoveGame::Script
