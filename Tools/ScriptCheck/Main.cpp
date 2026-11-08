//--------------------------------------------------------------
//! @file   Main.cpp
//! @brief  台本と Assets のデータを検証し、台本へ行IDを付けるコンソールツール
//!
//!   LoveGame.ScriptCheck [--stamp] [Assets フォルダ]
//!
//! 作業フォルダはリポジトリルートを想定する。問題が無ければ終了コード 0、あれば 1 を返す。
//--------------------------------------------------------------
#include <Script/CharacterTable.hpp>
#include <Script/GameConfig.hpp>
#include <Script/Parser.hpp>
#include <Script/Validator.hpp>

#include <cereal/archives/json.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>

#include <Windows.h>
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace LoveGame::Script;

namespace {
    std::string ToUtf8(const fs::path& path) {
        const std::u8string name = path.generic_u8string();
        return std::string(reinterpret_cast<const char*>(name.data()), name.size());
    }

    void PrintDiagnostics(const fs::path& path, const std::vector<Diagnostic>& diagnostics) {
        const std::string name = ToUtf8(path);
        for(const Diagnostic& d : diagnostics) {
            std::printf("%s(%d): %s\n", name.c_str(), d.line, d.message.c_str());
        }
    }

    int Report(const fs::path& path, const std::string& message) {
        std::printf("%s: %s\n", ToUtf8(path).c_str(), message.c_str());
        return 1;
    }

    //! @brief 目次やグループが指すパスを確認する。パスは Assets の親からの相対
    int CheckReferences(const fs::path& file, const fs::path& root, const char* key) {
        int errors = 0;
        try {
            std::ifstream                      stream(file, std::ios::binary);
            cereal::JSONInputArchive           archive(stream);
            std::map<std::string, std::string> entries;
            archive(cereal::make_nvp(key, entries));
            for(const auto& [name, target] : entries) {
                if(target != "null" && !fs::exists(root / target)) errors += Report(file, name + " の参照先がありません: " + target);
            }
        } catch(const std::exception& e) {
            errors += Report(file, e.what());
        }
        return errors;
    }

    //! @brief Prefab フォルダの JSON をすべて確認する。Instantiate は壊れた JSON で例外を投げるので、起動前に見つける
    int CheckPrefabs(const fs::path& prefabDir, const fs::path& root) {
        int             errors = 0;
        std::error_code ec;
        for(const fs::directory_entry& entry : fs::recursive_directory_iterator(prefabDir, ec)) {
            if(!entry.is_regular_file() || entry.path().extension() != ".json") continue;

            const std::string name = entry.path().filename().string();
            if(name == "Prefab.json") {
                errors += CheckReferences(entry.path(), root, "Components");
            } else if(name.size() > 11 && name.ends_with(".Group.json")) {
                errors += CheckReferences(entry.path(), root, "Entities");
            } else {
                try {
                    std::ifstream            stream(entry.path(), std::ios::binary);
                    cereal::JSONInputArchive archive(stream);
                } catch(const std::exception& e) {
                    errors += Report(entry.path(), e.what());
                }
            }
        }
        if(ec) errors += Report(prefabDir, "フォルダを読めません");
        return errors;
    }

    //! @brief 暦と自分磨きの定義を確認する
    int CheckCalendar(const fs::path& path, const CalendarConfig& calendar) {
        int errors = 0;
        if(calendar.TotalWeeks() <= 0) errors += Report(path, "暦に週がありません");
        for(const MonthDef& month : calendar.months) {
            if(month.weeks <= 0) errors += Report(path, month.name + " の週数は1以上にしてください");
        }
        if(calendar.actions.empty()) errors += Report(path, "自分磨きの行動がありません");
        for(const ActionDef& action : calendar.actions) {
            for(const auto& [stat, amount] : action.effects) {
                if(calendar.FindStat(stat) == nullptr) errors += Report(path, action.label + " の能力値が未定義です: " + stat);
            }
        }
        return errors;
    }

    //! @brief イベントの条件のラベルと変数名を確認する
    int CheckEventRequires(const fs::path& assets, const CharacterTable& characters, const CalendarConfig& calendar) {
        const fs::path path   = assets / "Data/characters.json";
        int            errors = 0;
        for(const CharacterDef& character : characters.characters) {
            std::string text;
            if(!ReadTextFile(assets / "Scripts" / character.script, text)) continue;    // 台本が無いことは別に報告済み

            const ParseResult result = Parse(character.script, text);
            for(const auto& [label, conditions] : character.eventRequires) {
                if(result.program.labels.count(label) == 0) {
                    errors += Report(path, character.id + " の eventRequires のラベルが台本にありません: " + label);
                }
                for(const auto& [variable, value] : conditions) {
                    const bool isStat      = variable.starts_with("stat.") && calendar.FindStat(variable.substr(5)) != nullptr;
                    const bool isAffection = variable.starts_with("affection.") && characters.FindById(variable.substr(10)) != nullptr;
                    if(!isStat && !isAffection) errors += Report(path, character.id + " の eventRequires の変数が未定義です: " + variable);
                }
            }
        }
        return errors;
    }
}    // namespace

int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);

    bool     stamp  = false;
    fs::path assets = "Assets";
    for(int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if(arg == "--stamp") stamp = true;
        else assets = arg;
    }
    const fs::path root = assets.parent_path();

    int errorCount = 0;

    CharacterTable characters;
    GameConfig     game;
    UiConfig       ui;
    StageConfig    stage;
    CalendarConfig calendar;
    std::string    error;
    if(!LoadCharacterTable(assets / "Data/characters.json", characters, error) || !LoadGameConfig(assets / "Data/game.json", game, error) ||
       !LoadUiConfig(assets / "Data/ui.json", ui, error) || !LoadStageConfig(assets / "Data/stage.json", stage, error) ||
       !LoadCalendarConfig(assets / "Data/calendar.json", calendar, error)) {
        std::printf("%s\n", error.c_str());
        return 1;
    }

    for(const CharacterDef& character : characters.characters) {
        if(!fs::exists(root / character.prefab)) errorCount += Report(assets / "Data/characters.json", "Prefab がありません: " + character.prefab);
        if(!fs::exists(assets / "Scripts" / character.script)) errorCount += Report(assets / "Data/characters.json", "台本がありません: " + character.script);
    }
    for(const auto& [name, material] : stage.stages) {
        if(!fs::exists(root / material)) errorCount += Report(assets / "Data/stage.json", "マテリアルがありません: " + material);
    }
    if(stage.stages.count(stage.defaultStage) == 0) errorCount += Report(assets / "Data/stage.json", "defaultStage が未定義です");
    if(stage.cameraShots.count(stage.defaultCamera) == 0) errorCount += Report(assets / "Data/stage.json", "defaultCamera が未定義です");

    errorCount += CheckCalendar(assets / "Data/calendar.json", calendar);
    errorCount += CheckEventRequires(assets, characters, calendar);
    errorCount += CheckPrefabs(assets / "Prefabs", root);

    std::vector<fs::path> scripts;
    std::error_code       ec;
    for(const fs::directory_entry& entry : fs::recursive_directory_iterator(assets / "Scripts", ec)) {
        if(entry.is_regular_file() && entry.path().extension() == ".txt") scripts.push_back(entry.path());
    }
    if(ec || scripts.empty()) {
        std::printf("台本が見つかりません: %s\n", ToUtf8(assets / "Scripts").c_str());
        return 1;
    }
    std::sort(scripts.begin(), scripts.end());

    for(const fs::path& path : scripts) {
        std::string text;
        if(!ReadTextFile(path, text)) {
            errorCount += Report(path, "読めません");
            continue;
        }

        if(stamp) {
            int               added   = 0;
            const std::string stamped = StampLineIds(text, added);
            if(added > 0) {
                std::ofstream file(path, std::ios::binary | std::ios::trunc);
                file.write(stamped.data(), static_cast<std::streamsize>(stamped.size()));
                text = stamped;
                std::printf("%s: 行IDを %d 件付けました\n", ToUtf8(path).c_str(), added);
            }
        }

        const ParseResult result = Parse(path.stem().string(), text);
        PrintDiagnostics(path, result.errors);
        errorCount += static_cast<int>(result.errors.size());

        const std::vector<Diagnostic> problems = Validate(result.program, characters, stage);
        PrintDiagnostics(path, problems);
        errorCount += static_cast<int>(problems.size());
    }

    std::printf("台本 %zu ファイルとデータを検証、問題 %d 件\n", scripts.size(), errorCount);
    return errorCount == 0 ? 0 : 1;
}
