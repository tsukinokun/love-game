//--------------------------------------------------------------
//! @file   GameScene.cpp
//! @brief  全シーン共通の土台の実装
//--------------------------------------------------------------
#include <Scene/GameScene.hpp>

#include <Save/SaveSystem.hpp>
#include <UI/TextLayout.hpp>
#include <UI/UIHelpers.hpp>
#include <UI/UITextComponent.hpp>

#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>
#include <Tsukino/Core/Log.hpp>
#include <Tsukino/EngineIntegration/ECS/System/CameraSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/FontRendererSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/InteractionSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/LightSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/ModelSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/SpriteRendererSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/TransformSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/UITweenSystem.hpp>
#include <Tsukino/EngineIntegration/EngineAPI.hpp>

#include <entt/entt.hpp>

#include <algorithm>
#include <memory>
#include <vector>

namespace LoveGame {
    namespace ECS = Tsukino::BuiltIn::ECS;

    namespace {
        constexpr const char* kBaseGroup    = "Assets/Prefabs/Common/Base.Group.json";
        constexpr const char* kSlotBackdrop = "Assets/Prefabs/Common/SlotBackdrop/Prefab.json";
        constexpr const char* kSlotButton   = "Assets/Prefabs/Common/DialogButton/Prefab.json";
    }    // namespace

    void GameScene::OnInitialize(Tsukino::EngineIntegration::EngineAPI&) {
        m_context = Registry().GetContext<Tsukino::EngineIntegration::EngineContext*>();

        m_scene.AddSystem(std::make_shared<ECS::InteractionSystem>(), -2);
        m_scene.AddSystem(std::make_shared<ECS::UITweenSystem>(), -1);
        m_scene.AddSystem(std::make_shared<ECS::TransformSystem>(), 0);
        m_scene.AddSystem(std::make_shared<ECS::CameraSystem>(), 5);
        m_scene.AddSystem(std::make_shared<ECS::FontRendererSystem>(), 9);
        m_scene.AddSystem(std::make_shared<ECS::SpriteRenderSystem>(), 10);
        m_scene.AddSystem(std::make_shared<ECS::ModelSystem>(), 10);
        m_scene.AddSystem(std::make_shared<ECS::LightSystem>(), 13);

        LoadLayout(kBaseGroup);
        m_fadeEntity = Find("Fade");

        Setup();
    }

    void GameScene::LoadLayout(const std::string& groupPath) {
        m_layout.merge(Prefabs().InstantiateGroup(groupPath, Registry()));

        auto view = Registry().View<UI::UITextComponent, ECS::FontComponent>();
        view.each([](entt::entity, const UI::UITextComponent& source, ECS::FontComponent& font) {
            font.text = TextLayout::ToWide(source.text);
        });
    }

    Tsukino::ECS::Entity GameScene::Find(const std::string& name) const {
        const auto it = m_layout.find(name);
        if(it != m_layout.end()) return it->second;
        Tsukino::Core::Log::Error("Entity not found in layout: " + name);
        return entt::null;
    }

    void GameScene::OnUpdate(Tsukino::EngineIntegration::EngineAPI& api, float deltaTime) {
        UpdateFade(deltaTime);

        if(m_nextScene != nullptr) {
            if(m_fadeAlpha >= 1.0f) {
                const char* next = m_nextScene;
                m_nextScene      = nullptr;
                api.ChangeScene(std::string(next));
            }
        } else if(m_fadeAlpha <= 0.0f) {
            Tick(deltaTime);
        }

        m_scene.Update(deltaTime);
    }

    void GameScene::UpdateFade(float deltaTime) {
        const float step = deltaTime / m_game.config.fadeSeconds;
        m_fadeAlpha      = std::clamp(m_fadeAlpha + (m_nextScene != nullptr ? step : -step), 0.0f, 1.0f);

        // 色は Prefab の値のまま、不透明度だけを動かす
        hlslpp::float4& tint = Registry().GetComponent<ECS::SpriteComponent>(m_fadeEntity).tintColor;
        tint.w               = m_fadeAlpha;
    }

    void GameScene::GoTo(const char* sceneName) { m_nextScene = sceneName; }

    void GameScene::OpenSlotPanel(bool saving) {
        m_slotSaving = saving;

        const Script::UiConfig&   ui = m_game.ui;
        std::vector<std::wstring> labels;
        for(int slot = 0; slot < m_game.config.slotCount; ++slot) {
            const std::string number = std::to_string(slot + 1);

            GameState   saved;
            std::string label;
            if(SaveSystem::Load(slot, saved)) {
                label = UI::Format(ui.slotFilled, "slot", number);
                label = UI::Format(label, "day", std::to_string(saved.day));
                label = UI::Format(label, "name", saved.playerName);
            } else {
                label = UI::Format(ui.slotEmpty, "slot", number);
            }
            labels.push_back(TextLayout::ToWide(label));
        }
        labels.push_back(TextLayout::ToWide(ui.slotClose));

        m_slotBackdrop = Prefabs().Instantiate(kSlotBackdrop, Registry());
        m_slotPanel.Show(m_scene, Prefabs(), kSlotButton, labels, ui, ui.slotCenterY);
    }

    bool GameScene::UpdateSlotPanel() {
        if(!m_slotPanel.IsOpen()) return false;

        const int index = m_slotPanel.ClickedIndex();
        if(index < 0) return true;

        if(index < m_game.config.slotCount) {
            if(m_slotSaving) {
                if(!SaveSystem::Save(index, m_game.state)) Tsukino::Core::Log::Error("Save failed.");
            } else if(SaveSystem::Load(index, m_game.state)) {
                GoTo(m_game.state.inConversation ? SceneName::kStage : SceneName::kCalendar);
            } else {
                return true;    // 空きスロットを押したときは開いたままにする
            }
        }
        m_slotPanel.Clear();
        m_scene.DestroyEntity(m_slotBackdrop);
        return true;
    }
}    // namespace LoveGame
