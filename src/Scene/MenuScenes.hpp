//--------------------------------------------------------------
//! @file   MenuScenes.hpp
//! @brief  タイトル、名前入力、カレンダーのシーン
//--------------------------------------------------------------
#pragma once
#include <Scene/GameScene.hpp>

#include <Script/CharacterTable.hpp>

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

        UI::ButtonList m_places;
    };
}    // namespace LoveGame
