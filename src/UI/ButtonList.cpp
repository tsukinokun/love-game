//--------------------------------------------------------------
//! @file   ButtonList.cpp
//! @brief  縦に並ぶボタンの実装
//--------------------------------------------------------------
#include <UI/ButtonList.hpp>

#include <UI/UIHelpers.hpp>

#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/UIRectComponent.hpp>

namespace LoveGame::UI {
    namespace ECS = Tsukino::BuiltIn::ECS;

    void ButtonList::Show(Tsukino::ECS::Scene& scene, Tsukino::Engine::ECS::Prefab::PrefabFactory& factory, const std::string& prefab,
                          const std::vector<std::wstring>& labels, const Script::UiConfig& config, float centerY) {
        Clear();
        m_scene = &scene;

        Tsukino::ECS::Registry& registry = scene.GetRegistry();
        for(const std::wstring& label : labels) {
            Tsukino::ECS::Entity button = factory.Instantiate(prefab, registry);
            SetText(registry, button, label);
            m_buttons.push_back(button);
        }
        if(m_buttons.empty()) return;

        // ボタンの高さは雛形の値を使う
        const float height = float(registry.GetComponent<ECS::UIRectComponent>(m_buttons.front()).size.y);
        const float step   = height + config.listGap;
        const float total  = step * static_cast<float>(m_buttons.size()) - config.listGap;

        float y = centerY - total * 0.5f + height * 0.5f;
        for(Tsukino::ECS::Entity button : m_buttons) {
            ECS::TransformComponent& transform = registry.GetComponent<ECS::TransformComponent>(button);
            transform.position                 = hlslpp::float3(config.listCenterX, y, 0.0f);
            transform.dirty                    = true;
            y += step;
        }
    }

    void ButtonList::Clear() {
        if(m_scene != nullptr) {
            for(Tsukino::ECS::Entity button : m_buttons) m_scene->DestroyEntity(button);
        }
        m_buttons.clear();
    }

    int ButtonList::ClickedIndex() const {
        for(std::size_t i = 0; i < m_buttons.size(); ++i) {
            if(WasClicked(m_scene->GetRegistry(), m_buttons[i])) return static_cast<int>(i);
        }
        return -1;
    }
}    // namespace LoveGame::UI
