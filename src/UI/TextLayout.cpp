//--------------------------------------------------------------
//! @file   TextLayout.cpp
//! @brief  文字コード変換と折り返しの実装
//--------------------------------------------------------------
#include <UI/TextLayout.hpp>

#include <Windows.h>

namespace LoveGame::TextLayout {
    namespace {
        float CharWidth(wchar_t c) {
            const bool halfWidthKana = c >= 0xFF61 && c <= 0xFF9F;
            return (c < 0x0100 || halfWidthKana) ? 0.5f : 1.0f;
        }
    }    // namespace

    std::wstring ToWide(std::string_view utf8) {
        if(utf8.empty()) return {};
        const int    size = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
        std::wstring wide(static_cast<std::size_t>(size), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), wide.data(), size);
        return wide;
    }

    std::string ToUtf8(std::wstring_view wide) {
        if(wide.empty()) return {};
        const int   size = WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
        std::string utf8(static_cast<std::size_t>(size), '\0');
        WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), utf8.data(), size, nullptr, nullptr);
        return utf8;
    }

    std::wstring Wrap(std::wstring_view text, float maxFullWidth, std::wstring_view noLineStart, std::wstring_view noLineEnd) {
        auto cannotStartLine = [&](wchar_t c) { return noLineStart.find(c) != std::wstring_view::npos; };
        auto cannotEndLine   = [&](wchar_t c) { return noLineEnd.find(c) != std::wstring_view::npos; };

        std::wstring result;
        result.reserve(text.size() + 8);

        std::size_t lineStart = 0;    // result の中で今の行が始まる位置
        float       width     = 0.0f;

        for(std::size_t i = 0; i < text.size(); ++i) {
            const wchar_t c = text[i];
            if(c == L'\r') continue;
            if(c == L'\n') {
                result.push_back(c);
                lineStart = result.size();
                width     = 0.0f;
                continue;
            }

            const float charWidth = CharWidth(c);
            if(width + charWidth > maxFullWidth && result.size() > lineStart) {
                // 改行位置を、行頭禁則と行末禁則に触れないところまで前へ戻す
                std::size_t breakAt = result.size();
                wchar_t     next    = c;
                while(breakAt > lineStart + 1 && (cannotStartLine(next) || cannotEndLine(result[breakAt - 1]))) {
                    --breakAt;
                    next = result[breakAt];
                }
                result.insert(breakAt, 1, L'\n');
                lineStart = breakAt + 1;

                width = 0.0f;
                for(std::size_t j = lineStart; j < result.size(); ++j) width += CharWidth(result[j]);
            }

            result.push_back(c);
            width += charWidth;
        }
        return result;
    }
}    // namespace LoveGame::TextLayout
