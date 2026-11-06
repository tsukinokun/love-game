//--------------------------------------------------------------
//! @file   GameConfig.cpp
//! @brief  Assets/Data の設定ファイルの読み込み
//--------------------------------------------------------------
#include <Script/GameConfig.hpp>

#include <cereal/archives/json.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include <fstream>

namespace LoveGame::Script {
    template <class Archive>
    void serialize(Archive& archive, Vec3& value) {
        archive(cereal::make_nvp("x", value.x), cereal::make_nvp("y", value.y), cereal::make_nvp("z", value.z));
    }

    template <class Archive>
    void serialize(Archive& archive, CameraShot& shot) {
        archive(cereal::make_nvp("position", shot.position), cereal::make_nvp("target", shot.target));
    }

    template <class Archive>
    void serialize(Archive& archive, GameConfig& config) {
        archive(cereal::make_nvp("lastDay", config.lastDay), cereal::make_nvp("affectionPerEvent", config.affectionPerEvent),
                cereal::make_nvp("maxNameLength", config.maxNameLength), cereal::make_nvp("slotCount", config.slotCount),
                cereal::make_nvp("fadeSeconds", config.fadeSeconds));
    }

    template <class Archive>
    void serialize(Archive& archive, UiConfig& config) {
        archive(cereal::make_nvp("charsPerSecond", config.charsPerSecond), cereal::make_nvp("fullWidthPerLine", config.fullWidthPerLine),
                cereal::make_nvp("noLineStart", config.noLineStart), cereal::make_nvp("noLineEnd", config.noLineEnd),
                cereal::make_nvp("listCenterX", config.listCenterX), cereal::make_nvp("listGap", config.listGap),
                cereal::make_nvp("choiceCenterY", config.choiceCenterY), cereal::make_nvp("slotCenterY", config.slotCenterY),
                cereal::make_nvp("placeCenterY", config.placeCenterY), cereal::make_nvp("statusY", config.statusY),
                cereal::make_nvp("statusStep", config.statusStep), cereal::make_nvp("candidateX", config.candidateX),
                cereal::make_nvp("candidateY", config.candidateY), cereal::make_nvp("dayFormat", config.dayFormat),
                cereal::make_nvp("statusFormat", config.statusFormat), cereal::make_nvp("slotFilled", config.slotFilled),
                cereal::make_nvp("slotEmpty", config.slotEmpty), cereal::make_nvp("slotClose", config.slotClose));
    }

    template <class Archive>
    void serialize(Archive& archive, StageConfig& config) {
        archive(cereal::make_nvp("defaultStage", config.defaultStage), cereal::make_nvp("defaultCamera", config.defaultCamera),
                cereal::make_nvp("cameraSpeed", config.cameraSpeed), cereal::make_nvp("stages", config.stages),
                cereal::make_nvp("positions", config.positions), cereal::make_nvp("cameraShots", config.cameraShots));
    }

    template <class Archive>
    void serialize(Archive& archive, PreviewShot& shot) {
        archive(cereal::make_nvp("name", shot.name), cereal::make_nvp("group", shot.group), cereal::make_nvp("position", shot.position),
                cereal::make_nvp("target", shot.target));
    }

    template <class Archive>
    void serialize(Archive& archive, PreviewConfig& config) {
        archive(cereal::make_nvp("shots", config.shots));
    }

    namespace {
        //! @brief ルートの項目 key を読む。項目が欠けていれば失敗にする
        template <class T>
        bool Load(const std::filesystem::path& path, const char* key, T& out, std::string& error) {
            std::ifstream file(path, std::ios::binary);
            if(!file) {
                error = "ファイルを開けません: " + path.string();
                return false;
            }
            try {
                cereal::JSONInputArchive archive(file);
                archive(cereal::make_nvp(key, out));
            } catch(const std::exception& e) {
                error = path.string() + ": " + e.what();
                return false;
            }
            return true;
        }
    }    // namespace

    bool LoadPreviewConfig(const std::filesystem::path& path, PreviewConfig& out, std::string& error) { return Load(path, "preview", out, error); }

    bool LoadGameConfig(const std::filesystem::path& path, GameConfig& out, std::string& error) { return Load(path, "game", out, error); }

    bool LoadUiConfig(const std::filesystem::path& path, UiConfig& out, std::string& error) { return Load(path, "ui", out, error); }

    bool LoadStageConfig(const std::filesystem::path& path, StageConfig& out, std::string& error) { return Load(path, "stage", out, error); }
}    // namespace LoveGame::Script
