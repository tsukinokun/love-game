//--------------------------------------------------------------
//! @file   UIHelpers.hpp
//! @brief  Prefab から作ったUIを操作する関数
//--------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/Registry/Registry.hpp>

#include <string>
#include <string_view>

namespace LoveGame::UI {
    //! @brief 文字を差し替える
    void SetText(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity, const std::wstring& text);

    //! @brief ボタンがこのフレームに押されたか
    bool WasClicked(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity button);

    //! @brief カーソルがボタンに重なっているか
    bool IsHovered(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity button);

    //! @brief 書式の {key} を値で置き換える
    std::string Format(std::string text, std::string_view key, const std::string& value);
}    // namespace LoveGame::UI
