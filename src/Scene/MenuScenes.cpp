//--------------------------------------------------------------
//! @file   MenuScenes.cpp
//! @brief  タイトル、名前入力、カレンダーのシーンの実装
//--------------------------------------------------------------
#include <Scene/MenuScenes.hpp>

#include <UI/TextLayout.hpp>
#include <UI/UIHelpers.hpp>

#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/BuiltIn/ECS/UI/UICanvas.hpp>
#include <Tsukino/Core/Input/InputSystem.hpp>
#include <Tsukino/Core/Input/TextInputSystem.hpp>

#include <Windows.h>

#include <string>
#include <vector>

namespace LoveGame {
    namespace ECS = Tsukino::BuiltIn::ECS;

    namespace {
        constexpr const char* kTitleLayout    = "Assets/Prefabs/Title/Scene.Group.json";
        constexpr const char* kNameLayout     = "Assets/Prefabs/NameEntry/Scene.Group.json";
        constexpr const char* kCalendarLayout = "Assets/Prefabs/Calendar/Scene.Group.json";
        constexpr const char* kPlaceButton    = "Assets/Prefabs/Common/Button/Prefab.json";
        constexpr const char* kStatusLabel    = "Assets/Prefabs/Calendar/Status/Prefab.json";

        int ReadVariable(const GameState& state, const std::string& name) {
            const auto it = state.variables.find(name);
            return it != state.variables.end() ? it->second : 0;
        }
    }    // namespace

    //--------------------------------------------------------------
    // タイトル
    //--------------------------------------------------------------
    void TitleScene::Setup() { LoadLayout(kTitleLayout); }

    void TitleScene::Tick(float) {
        if(UpdateSlotPanel()) return;

        if(UI::WasClicked(Registry(), Find("StartButton"))) {
            m_game.ResetState();
            GoTo(SceneName::kNameEntry);
        } else if(UI::WasClicked(Registry(), Find("ContinueButton"))) {
            OpenSlotPanel(false);
        } else if(UI::WasClicked(Registry(), Find("QuitButton"))) {
            PostQuitMessage(0);
        }
    }

    //--------------------------------------------------------------
    // 名前入力
    //--------------------------------------------------------------
    void NameEntryScene::Setup() {
        LoadLayout(kNameLayout);

        m_context->textInputSystem->SetText(L"");
        m_context->textInputSystem->Begin(static_cast<std::size_t>(m_game.config.maxNameLength));
    }

    void NameEntryScene::Tick(float) {
        Tsukino::Input::TextInputSystem& input = *m_context->textInputSystem;

        // 変換中の文字は確定済みの文字に続けて表示し、末尾にカーソルを置く
        UI::SetText(Registry(), Find("InputText"), input.GetText() + input.GetComposition() + L"_");

        const ECS::UICanvas& canvas = ECS::GetUICanvas(Registry());
        const hlslpp::float2 pixel  = canvas.ToPixel(hlslpp::float2(m_game.ui.candidateX, m_game.ui.candidateY));
        input.SetCandidatePosition(static_cast<int>(float(pixel.x)), static_cast<int>(float(pixel.y)));

        // 変換の確定に押した Enter は VK_PROCESSKEY になるので、ここへは来ない
        const bool pressedEnter = m_context->inputSystem->IsKeyPressed(Tsukino::Input::KeyCode::Enter) && input.GetComposition().empty();
        if(UI::WasClicked(Registry(), Find("ConfirmButton")) || pressedEnter) Confirm();
    }

    void NameEntryScene::Confirm() {
        Tsukino::Input::TextInputSystem& input = *m_context->textInputSystem;
        if(input.GetText().empty()) return;

        m_game.state.playerName = TextLayout::ToUtf8(input.GetText());
        input.End();
        GoTo(SceneName::kCalendar);
    }

    void NameEntryScene::OnExit() {
        if(m_context->textInputSystem->IsActive()) m_context->textInputSystem->End();
    }

    //--------------------------------------------------------------
    // カレンダー
    //--------------------------------------------------------------
    void CalendarScene::Setup() {
        LoadLayout(kCalendarLayout);

        const GameState&        state = m_game.state;
        const Script::UiConfig& ui    = m_game.ui;

        std::string day = UI::Format(ui.dayFormat, "day", std::to_string(state.day));
        day             = UI::Format(day, "lastDay", std::to_string(m_game.config.lastDay));
        UI::SetText(Registry(), Find("DayLabel"), TextLayout::ToWide(day));

        std::vector<std::wstring> labels;
        float                     y = ui.statusY;
        for(const Script::CharacterDef& character : m_game.characters.characters) {
            labels.push_back(TextLayout::ToWide(character.place));

            // 試作の確認用に好感度を見せる
            std::string status = UI::Format(ui.statusFormat, "name", character.name);
            status             = UI::Format(status, "affection", std::to_string(ReadVariable(state, "affection." + character.id)));

            Tsukino::ECS::Entity label = Prefabs().Instantiate(kStatusLabel, Registry());
            UI::SetText(Registry(), label, TextLayout::ToWide(status));

            ECS::TransformComponent& transform = Registry().GetComponent<ECS::TransformComponent>(label);
            transform.position.y               = y;
            transform.dirty                    = true;
            y += ui.statusStep;
        }
        m_places.Show(m_scene, Prefabs(), kPlaceButton, labels, ui, ui.placeCenterY);
    }

    void CalendarScene::Tick(float) {
        if(UpdateSlotPanel()) return;

        if(UI::WasClicked(Registry(), Find("SaveButton"))) {
            OpenSlotPanel(true);
            return;
        }

        const int index = m_places.ClickedIndex();
        if(index >= 0) StartEvent(m_game.characters.characters[static_cast<std::size_t>(index)]);
    }

    void CalendarScene::StartEvent(const Script::CharacterDef& character) {
        Script::Program program;
        if(!m_game.LoadProgram(character.script, program)) return;

        GameState& state     = m_game.state;
        const int  progress  = ReadVariable(state, "progress." + character.id);
        const int  affection = ReadVariable(state, "affection." + character.id);

        // 次のイベントが無いか、好感度が足りなければ日常会話にする
        const std::string next     = "event_" + std::to_string(progress + 1);
        const bool        hasEvent = program.labels.count(next) != 0 && affection >= progress * m_game.config.affectionPerEvent;

        state.inConversation = true;
        state.isEnding       = false;
        state.scriptFile     = character.script;
        state.startLabel     = hasEvent ? next : "daily";
        state.lineId.clear();
        state.stageName.clear();
        state.cameraShot.clear();
        state.shownActors.clear();
        GoTo(SceneName::kStage);
    }
}    // namespace LoveGame
