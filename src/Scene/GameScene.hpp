//--------------------------------------------------------------
//! @file   GameScene.hpp
//! @brief  全シーン共通の土台。システムの登録、Prefab の読み込み、画面の暗転を受け持つ
//--------------------------------------------------------------
#pragma once
#include <Game/Game.hpp>
#include <UI/ButtonList.hpp>

#include <Tsukino/Engine/ECS/Prefab/PrefabFactory.hpp>
#include <Tsukino/EngineIntegration/EngineContext.hpp>
#include <Tsukino/EngineIntegration/Scene/GameSceneBase.hpp>

#include <string>

namespace LoveGame {
    class GameScene : public Tsukino::EngineIntegration::GameSceneBase {
    public:
        explicit GameScene(Game& game) : m_game(game) {}

        void OnUpdate(Tsukino::EngineIntegration::EngineAPI& api, float deltaTime) final;
        void OnExit() override {}

    protected:
        void OnInitialize(Tsukino::EngineIntegration::EngineAPI& api) final;

        //! @brief シーン固有の初期化
        virtual void Setup() = 0;

        //! @brief 毎フレームの処理。暗転の間は呼ばれない
        virtual void Tick(float deltaTime) = 0;

        //! @brief グループの Prefab を生成し、文言を反映する
        //! @param groupPath [in] *.Group.json のパス
        void LoadLayout(const std::string& groupPath);

        //! @brief LoadLayout で作ったエンティティを名前で引く。無ければログへ出して無効な値を返す
        Tsukino::ECS::Entity Find(const std::string& name) const;

        //! @brief 暗転してから別のシーンへ移る
        void GoTo(const char* sceneName);

        //! @brief セーブ画面を開く
        //! @param saving [in] true でセーブ、false でロード
        void OpenSlotPanel(bool saving);

        //! @brief セーブ画面の処理
        //! @return セーブ画面が開いていれば true
        bool UpdateSlotPanel();

        Tsukino::ECS::Registry&                      Registry() { return m_scene.GetRegistry(); }
        Tsukino::Engine::ECS::Prefab::PrefabFactory& Prefabs() { return *m_context->prefabFactory; }

        Game&                                      m_game;
        Tsukino::EngineIntegration::EngineContext* m_context = nullptr;

    private:
        void UpdateFade(float deltaTime);

        Tsukino::Engine::ECS::Prefab::PrefabFactory::PrefabInstance m_layout;

        Tsukino::ECS::Entity m_fadeEntity{};
        float                m_fadeAlpha = 1.0f;
        const char*          m_nextScene = nullptr;

        UI::ButtonList       m_slotPanel;
        Tsukino::ECS::Entity m_slotBackdrop{};
        bool                 m_slotSaving = false;
    };
}    // namespace LoveGame
