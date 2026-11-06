//--------------------------------------------------------------
//! @file   MessageWindow.cpp
//! @brief  メッセージ窓の実装
//--------------------------------------------------------------
#include <UI/MessageWindow.hpp>

#include <UI/TextLayout.hpp>
#include <UI/UIHelpers.hpp>

#include <Tsukino/BuiltIn/ECS/Component/UIVisibilityComponent.hpp>

#include <algorithm>

namespace LoveGame::UI {
    namespace ECS = Tsukino::BuiltIn::ECS;

    void MessageWindow::Bind(Tsukino::ECS::Registry& registry, const Script::UiConfig& config, Tsukino::ECS::Entity namePanel,
                             Tsukino::ECS::Entity nameLabel, Tsukino::ECS::Entity messageText) {
        m_registry    = &registry;
        m_config      = &config;
        m_namePanel   = namePanel;
        m_nameLabel   = nameLabel;
        m_messageText = messageText;
        m_noLineStart = TextLayout::ToWide(config.noLineStart);
        m_noLineEnd   = TextLayout::ToWide(config.noLineEnd);
        SetNameVisible(false);
    }

    void MessageWindow::SetNameVisible(bool visible) {
        m_registry->GetComponent<ECS::UIVisibilityComponent>(m_namePanel).visible = visible;
        m_registry->GetComponent<ECS::UIVisibilityComponent>(m_nameLabel).visible = visible;
    }

    void MessageWindow::Show(const std::wstring& speaker, const std::wstring& text) {
        SetNameVisible(!speaker.empty());
        SetText(*m_registry, m_nameLabel, speaker);

        m_wrapped      = TextLayout::Wrap(text, m_config->fullWidthPerLine, m_noLineStart, m_noLineEnd);
        m_visibleChars = 0;
        m_elapsed      = 0.0f;
        Apply();
    }

    void MessageWindow::Update(float deltaTime) {
        if(!IsRevealing()) return;
        m_elapsed += deltaTime;
        m_visibleChars = std::min(m_wrapped.size(), static_cast<std::size_t>(m_elapsed * m_config->charsPerSecond));
        Apply();
    }

    void MessageWindow::RevealAll() {
        m_visibleChars = m_wrapped.size();
        Apply();
    }

    void MessageWindow::Apply() { SetText(*m_registry, m_messageText, m_wrapped.substr(0, m_visibleChars)); }
}    // namespace LoveGame::UI
