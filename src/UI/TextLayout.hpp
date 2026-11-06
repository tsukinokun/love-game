//--------------------------------------------------------------
//! @file   TextLayout.hpp
//! @brief  文字コード変換と、禁則処理つきの折り返し
//--------------------------------------------------------------
#pragma once
#include <string>
#include <string_view>

namespace LoveGame::TextLayout {
    //! @brief UTF-8 を描画用の wstring へ変換する
    std::wstring ToWide(std::string_view utf8);

    //! @brief wstring を UTF-8 へ変換する
    std::string ToUtf8(std::wstring_view wide);

    //! @brief 文章を折り返す。FontComponent は長い文を縮小してしまうので、事前に改行を入れる
    //! @param text         [in] 折り返す文章
    //! @param maxFullWidth [in] 1行の幅。全角を 1、半角を 0.5 として数える
    //! @param noLineStart  [in] 行頭に置けない文字
    //! @param noLineEnd    [in] 行末に置けない文字
    //! @return 改行を入れた文章
    std::wstring Wrap(std::wstring_view text, float maxFullWidth, std::wstring_view noLineStart, std::wstring_view noLineEnd);
}    // namespace LoveGame::TextLayout
