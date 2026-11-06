//--------------------------------------------------------------
//! @file   StageScene.cpp
//! @brief  3D舞台で会話を進めるシーンの実装
//--------------------------------------------------------------
#include <Scene/StageScene.hpp>

#include <UI/TextLayout.hpp>
#include <UI/UIHelpers.hpp>

#include <Tsukino/BuiltIn/ECS/Component/CameraComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/ModelComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/Core/Input/InputSystem.hpp>
#include <Tsukino/Core/Log.hpp>
#include <Tsukino/Core/Path.hpp>
#include <Tsukino/Engine/Asset/AssetManager.hpp>
#include <Tsukino/EngineIntegration/ECS/System/SkyAtmosphereSystem.hpp>

#include <algorithm>
#include <vector>

namespace LoveGame {
    namespace ECS = Tsukino::BuiltIn::ECS;
    using Tsukino::Input::KeyCode;

    namespace {
        constexpr const char* kLayout       = "Assets/Prefabs/Stage/Scene.Group.json";
        constexpr int         kSkyPriority  = 14;
        constexpr const char* kCameraEntity = "Camera";

        hlslpp::float3 ToFloat3(const Script::Vec3& v) { return hlslpp::float3(v.x, v.y, v.z); }
    }    // namespace

    void StageScene::Setup() {
        // IBLBaker は空の準備ができた最初のフレームに環境光を1回だけ焼く。
        // 空の無いシーンでこのシステムを動かすと未設定の空が焼かれて物体が真っ黒になるので、
        // 空を持つこのシーンでだけ登録する
        m_scene.AddSystem(std::make_shared<ECS::SkyAtmosphereSystem>(), kSkyPriority);

        LoadLayout(kLayout);
        m_message.Bind(Registry(), m_game.ui, Find("NamePanel"), Find("NameLabel"), Find("MessageText"));

        for(const Script::CharacterDef& character : m_game.characters.characters) {
            m_actors[character.id] = Prefabs().Instantiate(character.prefab, Registry());
        }

        GameState& state = m_game.state;

        // 舞台の状態はセーブから復元する。新しい会話では空のままなので既定値になる
        ApplyStage(state.stageName.empty() ? m_game.stage.defaultStage : state.stageName);
        const std::map<std::string, std::string> shown = state.shownActors;
        for(const auto& [id, position] : shown) ApplyActor(id, position);
        ApplyCamera(state.cameraShot.empty() ? m_game.stage.defaultCamera : state.cameraShot, true);

        if(!m_game.LoadProgram(state.scriptFile, m_program)) {
            Finish();
            return;
        }
        m_runner = std::make_unique<Script::Runner>(m_program, state.variables);

        const bool resumed = !state.lineId.empty() && m_runner->ResumeAt(state.lineId);
        if(!resumed) {
            if(!state.lineId.empty()) {
                Tsukino::Core::Log::Warn("Saved line was removed from the script. Restarting from the label: " + state.startLabel);
            }
            if(!m_runner->StartAt(state.startLabel)) {
                Tsukino::Core::Log::Error("Label not found: " + state.startLabel);
                Finish();
                return;
            }
        }
        Advance();
    }

    void StageScene::Tick(float deltaTime) {
        UpdateCamera(deltaTime);
        if(m_finished) return;
        if(UpdateSlotPanel()) return;

        const Tsukino::ECS::Entity saveButton = Find("SaveButton");
        if(UI::WasClicked(Registry(), saveButton)) {
            OpenSlotPanel(true);
            return;
        }

        if(m_choices.IsOpen()) {
            const int index = m_choices.ClickedIndex();
            if(index >= 0) {
                m_choices.Clear();
                if(m_runner->Choose(static_cast<std::size_t>(index))) Advance();
                else Finish();
            }
            return;
        }

        m_message.Update(deltaTime);

        const Tsukino::Input::InputSystem& input = *m_context->inputSystem;

        const bool clicked = input.IsKeyPressed(KeyCode::LButton) && !UI::IsHovered(Registry(), saveButton);
        if(clicked || input.IsKeyPressed(KeyCode::Enter) || input.IsKeyPressed(KeyCode::Space)) {
            if(m_message.IsRevealing()) m_message.RevealAll();
            else Advance();
        }
    }

    void StageScene::Advance() {
        while(true) {
            const Script::Op* op = m_runner->Next();
            if(op == nullptr || op->type == Script::OpType::End) {
                Finish();
                return;
            }

            if(op->type == Script::OpType::Command) {
                ExecuteCommand(*op);
                continue;
            }

            m_game.state.lineId = m_runner->CurrentLineId();

            if(op->type == Script::OpType::Text) {
                m_message.Show(Expand(op->speaker), Expand(op->text));
                return;
            }

            // 選択肢
            m_message.RevealAll();
            std::vector<std::wstring> labels;
            for(const Script::ChoiceOption& option : op->options) labels.push_back(Expand(option.text));
            m_choices.Show(m_scene, Prefabs(), "Assets/Prefabs/Common/Button/Prefab.json", labels, m_game.ui, m_game.ui.choiceCenterY);
            return;
        }
    }

    void StageScene::Finish() {
        m_finished = true;

        GameState& state     = m_game.state;
        state.inConversation = false;
        state.lineId.clear();

        if(state.isEnding) {
            GoTo(SceneName::kTitle);
            return;
        }

        ++state.day;
        if(state.day <= m_game.config.lastDay) {
            GoTo(SceneName::kCalendar);
            return;
        }

        // 最終日を終えたら、好感度が最も高い相手のエンディングへ
        const Script::CharacterDef* best      = nullptr;
        int                         bestValue = 0;
        for(const Script::CharacterDef& character : m_game.characters.characters) {
            const auto it    = state.variables.find("affection." + character.id);
            const int  value = it != state.variables.end() ? it->second : 0;
            if(best == nullptr || value > bestValue) {
                best      = &character;
                bestValue = value;
            }
        }
        if(best == nullptr) {
            GoTo(SceneName::kTitle);
            return;
        }

        state.inConversation = true;
        state.isEnding       = true;
        state.scriptFile     = best->script;
        state.startLabel     = "ending";
        state.stageName.clear();
        state.cameraShot.clear();
        state.shownActors.clear();
        GoTo(SceneName::kStage);
    }

    void StageScene::ExecuteCommand(const Script::Op& op) {
        if(op.command == "stage") ApplyStage(op.args[0]);
        else if(op.command == "show") ApplyActor(op.args[0], op.args[1]);
        else if(op.command == "hide") HideActor(op.args[0]);
        else if(op.command == "camera") ApplyCamera(op.args[0], false);
        else Tsukino::Core::Log::Info("Script command is not implemented yet: @" + op.command);
    }

    void StageScene::ApplyStage(const std::string& stageName) {
        const auto it = m_game.stage.stages.find(stageName);
        if(it == m_game.stage.stages.end()) {
            Tsukino::Core::Log::Warn("Unknown stage: " + stageName);
            return;
        }
        m_game.state.stageName = stageName;

        Tsukino::Asset::AssetHandle material = m_context->assetManager->Load(Tsukino::Core::Path(it->second));
        for(const char* name : {"Floor", "Wall"}) {
            std::vector<Tsukino::Asset::AssetRef>& materials = Registry().GetComponent<ECS::ModelComponent>(Find(name)).materials;
            materials.clear();
            materials.emplace_back(material);
        }
    }

    void StageScene::ApplyActor(const std::string& id, const std::string& position) {
        const auto actor = m_actors.find(id);
        const auto x     = m_game.stage.positions.find(position);
        if(actor == m_actors.end() || x == m_game.stage.positions.end()) {
            Tsukino::Core::Log::Warn("Unknown actor or position: " + id + " " + position);
            return;
        }

        ECS::TransformComponent& transform = Registry().GetComponent<ECS::TransformComponent>(actor->second);
        transform.position.x               = x->second;
        transform.dirty                    = true;
        Registry().GetComponent<ECS::ModelComponent>(actor->second).visible = true;

        m_game.state.shownActors[id] = position;
    }

    void StageScene::HideActor(const std::string& id) {
        const auto it = m_actors.find(id);
        if(it == m_actors.end()) return;
        Registry().GetComponent<ECS::ModelComponent>(it->second).visible = false;
        m_game.state.shownActors.erase(id);
    }

    void StageScene::ApplyCamera(const std::string& shot, bool immediate) {
        const auto it = m_game.stage.cameraShots.find(shot);
        if(it == m_game.stage.cameraShots.end()) {
            Tsukino::Core::Log::Warn("Unknown camera shot: " + shot);
            return;
        }
        m_game.state.cameraShot = shot;
        m_cameraPosition        = ToFloat3(it->second.position);
        m_cameraTarget          = ToFloat3(it->second.target);

        if(immediate) {
            const Tsukino::ECS::Entity entity = Find(kCameraEntity);

            ECS::TransformComponent& transform = Registry().GetComponent<ECS::TransformComponent>(entity);
            transform.position                 = m_cameraPosition;
            transform.dirty                    = true;

            ECS::CameraComponent& camera = Registry().GetComponent<ECS::CameraComponent>(entity);
            camera.lookAtTarget          = m_cameraTarget;
            camera.dirty                 = true;
        }
    }

    void StageScene::UpdateCamera(float deltaTime) {
        const float                t      = std::min(1.0f, deltaTime * m_game.stage.cameraSpeed);
        const Tsukino::ECS::Entity entity = Find(kCameraEntity);

        ECS::TransformComponent& transform = Registry().GetComponent<ECS::TransformComponent>(entity);
        transform.position                 = transform.position + (m_cameraPosition - transform.position) * t;
        transform.dirty                    = true;

        ECS::CameraComponent& camera = Registry().GetComponent<ECS::CameraComponent>(entity);
        camera.lookAtTarget          = camera.lookAtTarget + (m_cameraTarget - camera.lookAtTarget) * t;
        camera.dirty                 = true;
    }

    std::wstring StageScene::Expand(const std::string& text) const {
        return TextLayout::ToWide(UI::Format(text, "player", m_game.state.playerName));
    }
}    // namespace LoveGame
