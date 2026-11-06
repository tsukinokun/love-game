//--------------------------------------------------------------
//! @file   Game.cpp
//! @brief  ゲーム全体の状態の実装
//--------------------------------------------------------------
#include <Game/Game.hpp>

#include <Script/Parser.hpp>
#include <Script/Validator.hpp>

#include <Tsukino/Core/Log.hpp>

#include <filesystem>

namespace LoveGame {
    bool Game::LoadData(std::string& error) {
        return Script::LoadCharacterTable("Assets/Data/characters.json", characters, error) &&
               Script::LoadGameConfig("Assets/Data/game.json", config, error) && Script::LoadUiConfig("Assets/Data/ui.json", ui, error) &&
               Script::LoadStageConfig("Assets/Data/stage.json", stage, error) &&
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
}    // namespace LoveGame
