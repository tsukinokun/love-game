//--------------------------------------------------------------
//! @file   UIHelpers.cpp
//! @brief  Prefab から作ったUIを操作する関数の実装
//--------------------------------------------------------------
#include <UI/UIHelpers.hpp>

#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/PointerTargetComponent.hpp>

namespace LoveGame::UI {
    namespace ECS = Tsukino::BuiltIn::ECS;

    void SetText(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity, const std::wstring& text) {
        registry.GetComponent<ECS::FontComponent>(entity).text = text;
    }

    bool WasClicked(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity button) {
        return registry.IsValid(button) && registry.GetComponent<ECS::PointerTargetComponent>(button).clicked;
    }

    bool IsHovered(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity button) {
        return registry.IsValid(button) && registry.GetComponent<ECS::PointerTargetComponent>(button).hovered;
    }

    std::string Format(std::string text, std::string_view key, const std::string& value) {
        const std::string token = "{" + std::string(key) + "}";
        std::size_t       at    = 0;
        while((at = text.find(token, at)) != std::string::npos) {
            text.replace(at, token.size(), value);
            at += value.size();
        }
        return text;
    }
}    // namespace LoveGame::UI
