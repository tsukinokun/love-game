//--------------------------------------------------------------
//! @file   Parser.hpp
//! @brief  台本テキストの解析と行IDの付与
//--------------------------------------------------------------
#pragma once
#include <Script/Program.hpp>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace LoveGame::Script {
    //! @brief 解析結果
    struct ParseResult {
        Program                 program;
        std::vector<Diagnostic> errors;
    };

    //--------------------------------------------------------------
    //! @brief  台本テキストを解析する
    //! @param  name [in] 台本の名前（診断と保存に使う）
    //! @param  text [in] UTF-8 の台本本文
    //! @return 解析結果。errors が空でなくても解析できた範囲の program を返す
    //--------------------------------------------------------------
    ParseResult Parse(const std::string& name, std::string_view text);

    //--------------------------------------------------------------
    //! @brief  行IDのない台詞と選択肢へ行IDを付ける
    //! @param  text  [in]  UTF-8 の台本本文
    //! @param  added [out] 付けた行IDの数
    //! @return 行IDを付けた台本本文。改行コードは元のまま
    //--------------------------------------------------------------
    std::string StampLineIds(std::string_view text, int& added);

    //--------------------------------------------------------------
    //! @brief  テキストファイルを読む（UTF-8 の BOM は取り除く）
    //! @param  path [in]  読み込むファイル
    //! @param  out  [out] 読み込んだ本文
    //! @return 読めたら true
    //--------------------------------------------------------------
    bool ReadTextFile(const std::filesystem::path& path, std::string& out);
}    // namespace LoveGame::Script
