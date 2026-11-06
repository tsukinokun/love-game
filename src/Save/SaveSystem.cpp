//--------------------------------------------------------------
//! @file   SaveSystem.cpp
//! @brief  セーブデータの読み書きの実装
//--------------------------------------------------------------
#include <Save/SaveSystem.hpp>

#include <Tsukino/Core/Log.hpp>

#include <cereal/archives/json.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>

#include <Windows.h>
#include <ShlObj.h>

#include <filesystem>
#include <fstream>

namespace LoveGame {
    template <class Archive>
    void serialize(Archive& archive, GameState& state) {
        archive(cereal::make_nvp("playerName", state.playerName), cereal::make_nvp("day", state.day),
                cereal::make_nvp("variables", state.variables), cereal::make_nvp("inConversation", state.inConversation),
                cereal::make_nvp("isEnding", state.isEnding), cereal::make_nvp("scriptFile", state.scriptFile),
                cereal::make_nvp("startLabel", state.startLabel), cereal::make_nvp("lineId", state.lineId),
                cereal::make_nvp("stageName", state.stageName), cereal::make_nvp("cameraShot", state.cameraShot),
                cereal::make_nvp("shownActors", state.shownActors));
    }
}    // namespace LoveGame

namespace LoveGame::SaveSystem {
    namespace {
        namespace fs = std::filesystem;

        //! @brief 保存先のフォルダ。インストール先は書き込めないことがあるので使わない
        fs::path SaveDirectory() {
            PWSTR    raw = nullptr;
            fs::path directory;
            if(SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &raw))) {
                directory = fs::path(raw) / L"LoveGame" / L"Saves";
            }
            CoTaskMemFree(raw);
            return directory;
        }

        fs::path SlotPath(int slot) { return SaveDirectory() / (L"slot" + std::to_wstring(slot + 1) + L".json"); }
    }    // namespace

    bool Save(int slot, const GameState& state) {
        const fs::path path = SlotPath(slot);
        if(path.empty()) return false;

        std::error_code ec;
        fs::create_directories(path.parent_path(), ec);

        // 一時ファイルへ書き切ってから置き換える
        fs::path temporary = path;
        temporary += L".tmp";
        try {
            std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
            if(!file) return false;
            cereal::JSONOutputArchive archive(file);
            GameState                 copy = state;
            archive(cereal::make_nvp("state", copy));
        } catch(const std::exception& e) {
            Tsukino::Core::Log::Error(std::string("Failed to write save data: ") + e.what());
            return false;
        }

        if(!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            Tsukino::Core::Log::Error("Failed to replace save file. Previous data is kept.");
            return false;
        }
        return true;
    }

    bool Load(int slot, GameState& state) {
        std::ifstream file(SlotPath(slot), std::ios::binary);
        if(!file) return false;
        try {
            cereal::JSONInputArchive archive(file);
            GameState                loaded;
            archive(cereal::make_nvp("state", loaded));
            state = std::move(loaded);
        } catch(const std::exception& e) {
            Tsukino::Core::Log::Error(std::string("Failed to read save data: ") + e.what());
            return false;
        }
        return true;
    }
}    // namespace LoveGame::SaveSystem
