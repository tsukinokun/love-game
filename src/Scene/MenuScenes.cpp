//--------------------------------------------------------------
//! @file   MenuScenes.cpp
//! @brief  タイトル、名前入力、カレンダーのシーンの実装
//--------------------------------------------------------------
#include <Scene/MenuScenes.hpp>

#include <UI/TextLayout.hpp>
#include <UI/UIHelpers.hpp>

#include <algorithm>

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

        const std::string date = m_game.FormatDate(state.week);
        std::string       week = UI::Format(ui.weekFormat, "date", date);
        week                   = UI::Format(week, "current", std::to_string(state.week));
        week                   = UI::Format(week, "total", std::to_string(m_game.calendar.TotalWeeks()));
        UI::SetText(Registry(), Find("DayLabel"), TextLayout::ToWide(week));

        if(!state.appointments.empty()) UI::SetText(Registry(), Find("Appointment"), TextLayout::ToWide(AppointmentText()));

        // 能力値は左、好感度（試作の確認用）は右に並べる
        float statY = ui.statusY;
        for(const Script::StatDef& stat : m_game.calendar.stats) {
            m_statLabels.push_back(CreateStatus(ui.statX, statY));
            statY += ui.statusStep;
        }
        RefreshStats();

        float                     y = ui.statusY;
        std::vector<std::wstring> places;
        for(const Script::CharacterDef& character : m_game.characters.characters) {
            places.push_back(TextLayout::ToWide(character.place));

            std::string status = UI::Format(ui.statusFormat, "name", character.name);
            status             = UI::Format(status, "affection", std::to_string(state.Get("affection." + character.id)));
            UI::SetText(Registry(), CreateStatus(ui.affectionX, y), TextLayout::ToWide(status));
            y += ui.statusStep;
        }

        std::vector<std::wstring> actions;
        for(const Script::ActionDef& action : m_game.calendar.actions) actions.push_back(TextLayout::ToWide(action.label));

        m_actions.Show(m_scene, Prefabs(), kPlaceButton, actions, ui, ui.actionCenterX, ui.placeCenterY);
        m_places.Show(m_scene, Prefabs(), kPlaceButton, places, ui, ui.placeCenterX, ui.placeCenterY);
    }

    Tsukino::ECS::Entity CalendarScene::CreateStatus(float x, float y) {
        Tsukino::ECS::Entity label = Prefabs().Instantiate(kStatusLabel, Registry());

        ECS::TransformComponent& transform = Registry().GetComponent<ECS::TransformComponent>(label);
        transform.position.x               = x;
        transform.position.y               = y;
        transform.dirty                    = true;
        return label;
    }

    void CalendarScene::RefreshStats() {
        for(std::size_t i = 0; i < m_statLabels.size(); ++i) {
            const Script::StatDef& stat = m_game.calendar.stats[i];

            std::string text = UI::Format(m_game.ui.statFormat, "name", stat.name);
            text             = UI::Format(text, "value", std::to_string(m_game.state.Get("stat." + stat.id)));
            UI::SetText(Registry(), m_statLabels[i], TextLayout::ToWide(text));
        }
    }

    std::string CalendarScene::AppointmentText() const {
        const Appointment& next = m_game.state.appointments.front();

        std::string name;
        for(const Script::CharacterDef& character : m_game.characters.characters) {
            if(character.script == next.scriptFile) name = character.name;
        }
        return UI::Format(UI::Format(m_game.ui.appointmentFormat, "date", m_game.FormatDate(next.week)), "name", name);
    }

    void CalendarScene::Tick(float) {
        // 結果の表示中はセーブを受け付けない。能力値を増やした状態のまま同じ週をやり直せてしまう
        if(m_showingResult) {
            const Tsukino::Input::InputSystem& input = *m_context->inputSystem;
            if(input.IsKeyPressed(Tsukino::Input::KeyCode::LButton) || input.IsKeyPressed(Tsukino::Input::KeyCode::Enter) ||
               input.IsKeyPressed(Tsukino::Input::KeyCode::Space)) {
                m_showingResult = false;
                GoTo(m_game.FinishWeek());
            }
            return;
        }

        if(UpdateSlotPanel()) return;

        if(UI::WasClicked(Registry(), Find("SaveButton"))) {
            OpenSlotPanel(true);
            return;
        }

        const int action = m_actions.ClickedIndex();
        if(action >= 0) {
            PerformAction(m_game.calendar.actions[static_cast<std::size_t>(action)]);
            return;
        }

        const int index = m_places.ClickedIndex();
        if(index >= 0) StartEvent(m_game.characters.characters[static_cast<std::size_t>(index)]);
    }

    void CalendarScene::PerformAction(const Script::ActionDef& action) {
        const Script::UiConfig& ui = m_game.ui;

        std::wstring text = TextLayout::Wrap(TextLayout::ToWide(action.result), ui.fullWidthPerLine, TextLayout::ToWide(ui.noLineStart),
                                             TextLayout::ToWide(ui.noLineEnd));

        for(const auto& [id, amount] : action.effects) {
            int&      value  = m_game.state.variables["stat." + id];
            const int before = value;
            value            = std::min(value + amount, m_game.config.statMax);

            const Script::StatDef* stat   = m_game.calendar.FindStat(id);
            std::string            effect = UI::Format(ui.effectFormat, "name", stat != nullptr ? stat->name : id);
            effect                        = UI::Format(effect, "value", std::to_string(value - before));
            text += L"\n" + TextLayout::ToWide(effect);
        }
        text += L"\n\n" + TextLayout::ToWide(ui.resultHint);

        m_actions.Clear();
        m_places.Clear();
        for(const char* name : {"Prompt", "Appointment", "ActionHeader", "PlaceHeader"}) UI::SetText(Registry(), Find(name), L"");
        UI::SetText(Registry(), Find("Result"), text);
        RefreshStats();
        m_showingResult = true;
    }

    void CalendarScene::StartEvent(const Script::CharacterDef& character) {
        Script::Program program;
        if(!m_game.LoadProgram(character.script, program)) return;

        const int         progress = m_game.state.Get("progress." + character.id);
        const std::string next     = "event_" + std::to_string(progress + 1);

        // 次のイベントが無いか、条件を満たさなければ日常会話にする
        bool hasEvent = program.labels.count(next) != 0;
        const auto requires_it = character.eventRequires.find(next);
        if(hasEvent && requires_it != character.eventRequires.end()) {
            for(const auto& [variable, value] : requires_it->second) {
                if(m_game.state.Get(variable) < value) hasEvent = false;
            }
        }

        m_game.BeginConversation(character.script, hasEvent ? next : "daily", false);
        GoTo(SceneName::kStage);
    }
}    // namespace LoveGame
