//--------------------------------------------------------------
//! @file   MessageWindow.hpp
//! @brief  台詞を表示するメッセージ窓。見た目は Prefab、文字送りの値は ui.json が持つ
//--------------------------------------------------------------
#pragma once
#include <Script/GameConfig.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>

#include <cstddef>
#include <string>

namespace LoveGame::UI {
    class MessageWindow {
    public:
        //! @brief 窓の部品を受け取る
        void Bind(Tsukino::ECS::Registry& registry, const Script::UiConfig& config, Tsukino::ECS::Entity namePanel,
                  Tsukino::ECS::Entity nameLabel, Tsukino::ECS::Entity messageText);

        //! @brief 文章の文字送りを始める
        //! @param speaker [in] 話者。空なら地の文
        //! @param text    [in] 本文
        void Show(const std::wstring& speaker, const std::wstring& text);

        //! @brief 文字送りを進める
        void Update(float deltaTime);

        //! @brief 文字送りの途中か
        bool IsRevealing() const { return m_visibleChars < m_wrapped.size(); }

        //! @brief 文章を最後まで表示する
        void RevealAll();

    private:
        void SetNameVisible(bool visible);
        void Apply();

        Tsukino::ECS::Registry* m_registry = nullptr;
        const Script::UiConfig* m_config   = nullptr;
        Tsukino::ECS::Entity    m_namePanel{};
        Tsukino::ECS::Entity    m_nameLabel{};
        Tsukino::ECS::Entity    m_messageText{};
        std::wstring            m_noLineStart;
        std::wstring            m_noLineEnd;
        std::wstring            m_wrapped;
        std::size_t             m_visibleChars = 0;
        float                   m_elapsed      = 0.0f;
    };
}    // namespace LoveGame::UI
