//--------------------------------------------------------------
//! @file   UITextComponent.hpp
//! @brief  Prefab に書く文言。FontComponent::text は Prefab に保存されないので、これを介して渡す
//--------------------------------------------------------------
#pragma once
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>

#include <string>

namespace LoveGame::UI {
    struct UITextComponent {
        std::string text;    // 表示する文言（UTF-8）
    };

    template <class Archive>
    void serialize(Archive& archive, UITextComponent& component) {
        archive(cereal::make_nvp("text", component.text));
    }
}    // namespace LoveGame::UI
