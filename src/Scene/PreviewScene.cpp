//--------------------------------------------------------------
//! @file   PreviewScene.cpp
//! @brief  モデル確認シーンの実装
//--------------------------------------------------------------
#include <Scene/PreviewScene.hpp>

#include <UI/TextLayout.hpp>
#include <UI/UIHelpers.hpp>

#include <Tsukino/BuiltIn/ECS/Component/CameraComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/Core/Input/InputSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/SkyAtmosphereSystem.hpp>

#include <memory>

namespace LoveGame {
    namespace ECS = Tsukino::BuiltIn::ECS;

    namespace {
        constexpr const char* kCommonLayout = "Assets/Prefabs/Preview/Common.Group.json";
        constexpr int         kSkyPriority  = 14;

        hlslpp::float3 ToFloat3(const Script::Vec3& v) { return hlslpp::float3(v.x, v.y, v.z); }
    }    // namespace

    void PreviewScene::Setup() {
        m_scene.AddSystem(std::make_shared<ECS::SkyAtmosphereSystem>(), kSkyPriority);

        const Script::PreviewShot& shot = m_game.preview.shots[static_cast<std::size_t>(m_game.previewIndex)];

        LoadLayout(kCommonLayout);
        LoadLayout(shot.group);
        UI::SetText(Registry(), Find("Label"), TextLayout::ToWide(shot.name));

        const Tsukino::ECS::Entity entity = Find("Camera");

        ECS::TransformComponent& transform = Registry().GetComponent<ECS::TransformComponent>(entity);
        transform.position                 = ToFloat3(shot.position);
        transform.dirty                    = true;

        ECS::CameraComponent& camera = Registry().GetComponent<ECS::CameraComponent>(entity);
        camera.lookAtTarget          = ToFloat3(shot.target);
        camera.dirty                 = true;
    }

    void PreviewScene::Tick(float) {
        // 数字キーで場面を切り替える
        const Tsukino::Input::InputSystem& input = *m_context->inputSystem;
        for(std::size_t i = 0; i < m_game.preview.shots.size(); ++i) {
            const auto key = static_cast<Tsukino::Input::KeyCode>('1' + i);
            if(input.IsKeyPressed(key) && static_cast<int>(i) != m_game.previewIndex) {
                m_game.previewIndex = static_cast<int>(i);
                GoTo(SceneName::kPreview);
                return;
            }
        }
    }
}    // namespace LoveGame
