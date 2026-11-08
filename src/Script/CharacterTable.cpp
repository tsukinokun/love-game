//--------------------------------------------------------------
//! @file   CharacterTable.cpp
//! @brief  登場人物の定義の読み込み
//--------------------------------------------------------------
#include <Script/CharacterTable.hpp>

#include <cereal/archives/json.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include <fstream>

namespace LoveGame::Script {
    template <class Archive>
    void serialize(Archive& archive, CharacterDef& def) {
        archive(cereal::make_nvp("id", def.id), cereal::make_nvp("name", def.name), cereal::make_nvp("place", def.place),
                cereal::make_nvp("prefab", def.prefab), cereal::make_nvp("script", def.script), cereal::make_nvp("eventRequires", def.eventRequires));
    }

    const CharacterDef* CharacterTable::FindById(const std::string& id) const {
        for(const CharacterDef& def : characters) {
            if(def.id == id) return &def;
        }
        return nullptr;
    }

    bool LoadCharacterTable(const std::filesystem::path& path, CharacterTable& out, std::string& error) {
        std::ifstream file(path, std::ios::binary);
        if(!file) {
            error = "ファイルを開けません: " + path.string();
            return false;
        }
        try {
            cereal::JSONInputArchive archive(file);
            archive(cereal::make_nvp("characters", out.characters));
        } catch(const std::exception& e) {
            error = path.string() + ": " + e.what();
            return false;
        }
        return true;
    }
}    // namespace LoveGame::Script
