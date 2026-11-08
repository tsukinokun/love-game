//--------------------------------------------------------------
//! @file   Game.cpp
//! @brief  ゲーム全体の状態の実装
//--------------------------------------------------------------
#include <Game/Game.hpp>

#include <Script/Parser.hpp>
#include <Script/Validator.hpp>
#include <UI/UIHelpers.hpp>

#include <Tsukino/Core/Log.hpp>

#include <algorithm>
#include <filesystem>

namespace LoveGame {
    int GameState::Get(const std::string& name) const {
        const auto it = variables.find(name);
        return it != variables.end() ? it->second : 0;
    }

    bool Game::LoadData(std::string& error) {
        return Script::LoadCharacterTable("Assets/Data/characters.json", characters, error) &&
               Script::LoadGameConfig("Assets/Data/game.json", config, error) && Script::LoadUiConfig("Assets/Data/ui.json", ui, error) &&
               Script::LoadStageConfig("Assets/Data/stage.json", stage, error) &&
               Script::LoadCalendarConfig("Assets/Data/calendar.json", calendar, error) &&
               Script::LoadPreviewConfig("Assets/Data/preview.json", preview, error);
    }

    bool Game::LoadProgram(const std::string& file, Script::Program& program) const {
        const std::filesystem::path path = std::filesystem::path("Assets/Scripts") / file;

        std::string text;
        if(!Script::ReadTextFile(path, text)) {
            Tsukino::Core::Log::Error("Script file not found: " + path.string());
            return false;
        }

        Script::ParseResult                   result   = Script::Parse(path.stem().string(), text);
        const std::vector<Script::Diagnostic> problems = Script::Validate(result.program, characters, stage);
        for(const Script::Diagnostic& d : result.errors) {
            Tsukino::Core::Log::Error(file + "(" + std::to_string(d.line) + "): " + d.message);
        }
        for(const Script::Diagnostic& d : problems) {
            Tsukino::Core::Log::Warn(file + "(" + std::to_string(d.line) + "): " + d.message);
        }

        program = std::move(result.program);
        return result.errors.empty();
    }

    void Game::ResetState() { state = GameState{}; }

    void Game::BeginConversation(const std::string& script, const std::string& label, bool isEnding) {
        state.inConversation = true;
        state.isEnding       = isEnding;
        state.scriptFile     = script;
        state.startLabel     = label;
        state.lineId.clear();
        state.stageName.clear();
        state.cameraShot.clear();
        state.shownActors.clear();
    }

    const char* Game::FinishWeek() {
        ++state.week;

        // 全週を終えたら、好感度が最も高い相手のエンディングへ
        if(state.week > calendar.TotalWeeks()) {
            const Script::CharacterDef* best      = nullptr;
            int                         bestValue = 0;
            for(const Script::CharacterDef& character : characters.characters) {
                const int value = state.Get("affection." + character.id);
                if(best == nullptr || value > bestValue) {
                    best      = &character;
                    bestValue = value;
                }
            }
            if(best == nullptr) return SceneName::kTitle;

            BeginConversation(best->script, "ending", true);
            return SceneName::kStage;
        }

        // 約束の週なら、選択を挟まずにデートを始める
        const auto it = std::find_if(state.appointments.begin(), state.appointments.end(),
                                     [this](const Appointment& appointment) { return appointment.week == state.week; });
        if(it != state.appointments.end()) {
            const Appointment appointment = *it;
            state.appointments.erase(it);
            BeginConversation(appointment.scriptFile, appointment.label, false);
            return SceneName::kStage;
        }
        return SceneName::kCalendar;
    }

    bool Game::AddAppointment(const std::string& script, const std::string& label, int weeksLater) {
        int week = state.week + weeksLater;
        while(std::any_of(state.appointments.begin(), state.appointments.end(),
                          [week](const Appointment& appointment) { return appointment.week == week; })) {
            ++week;
        }
        if(week > calendar.TotalWeeks()) return false;

        state.appointments.push_back({week, script, label});
        std::sort(state.appointments.begin(), state.appointments.end(),
                  [](const Appointment& a, const Appointment& b) { return a.week < b.week; });
        return true;
    }

    std::string Game::FormatDate(int week) const {
        int remaining = week;
        for(const Script::MonthDef& month : calendar.months) {
            if(remaining <= month.weeks) {
                return UI::Format(UI::Format(ui.dateFormat, "month", month.name), "week", std::to_string(remaining));
            }
            remaining -= month.weeks;
        }
        return std::string();
    }
}    // namespace LoveGame
