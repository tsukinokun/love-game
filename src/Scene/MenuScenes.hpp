//--------------------------------------------------------------
//! @file   MenuScenes.hpp
//! @brief  タイトル、名前入力、カレンダーのシーン
//--------------------------------------------------------------
#pragma once
#include <Scene/GameScene.hpp>

#include <Script/CharacterTable.hpp>
#include <Script/GameConfig.hpp>

#include <string>
#include <vector>

namespace LoveGame {
    class TitleScene : public GameScene {
    public:
        using GameScene::GameScene;

    protected:
        void Setup() override;
        void Tick(float deltaTime) override;
    };

    class NameEntryScene : public GameScene {
    public:
        using GameScene::GameScene;
        void OnExit() override;

    protected:
        void Setup() override;
        void Tick(float deltaTime) override;

    private:
        void Confirm();
    };

    class CalendarScene : public GameScene {
    public:
        using GameScene::GameScene;

    protected:
        void Setup() override;
        void Tick(float deltaTime) override;

    private:
        void StartEvent(const Script::CharacterDef& character);
        void PerformAction(const Script::ActionDef& action);
        void RefreshStats();
        std::string AppointmentText() const;
        Tsukino::ECS::Entity CreateStatus(float x, float y);

        UI::ButtonList                    m_actions;
        UI::ButtonList                    m_places;
        std::vector<Tsukino::ECS::Entity> m_statLabels;
        bool                              m_showingResult = false;    // 自分磨きの結果を表示中か
    };
}    // namespace LoveGame
