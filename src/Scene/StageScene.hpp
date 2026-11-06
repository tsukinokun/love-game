//--------------------------------------------------------------
//! @file   StageScene.hpp
//! @brief  3D舞台で会話を進めるシーン
//--------------------------------------------------------------
#pragma once
#include <Scene/GameScene.hpp>

#include <Script/Program.hpp>
#include <Script/Runner.hpp>
#include <UI/MessageWindow.hpp>

#include <hlsl++.h>

#include <map>
#include <memory>
#include <string>

namespace LoveGame {
    class StageScene : public GameScene {
    public:
        using GameScene::GameScene;

    protected:
        void Setup() override;
        void Tick(float deltaTime) override;

    private:
        void Advance();
        void Finish();
        void ExecuteCommand(const Script::Op& op);

        void ApplyStage(const std::string& stageName);
        void ApplyActor(const std::string& id, const std::string& position);
        void HideActor(const std::string& id);
        void ApplyCamera(const std::string& shot, bool immediate);
        void UpdateCamera(float deltaTime);

        std::wstring Expand(const std::string& text) const;

        Script::Program                 m_program;
        std::unique_ptr<Script::Runner> m_runner;
        bool                            m_finished = false;

        UI::MessageWindow m_message;
        UI::ButtonList    m_choices;

        std::map<std::string, Tsukino::ECS::Entity> m_actors;

        hlslpp::float3 m_cameraPosition = hlslpp::float3(0.0f, 0.0f, 0.0f);
        hlslpp::float3 m_cameraTarget   = hlslpp::float3(0.0f, 0.0f, 0.0f);
    };
}    // namespace LoveGame
