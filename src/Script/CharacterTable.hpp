//--------------------------------------------------------------
//! @file   CharacterTable.hpp
//! @brief  登場人物の定義。ゲーム本体と検証ツールで共有する
//--------------------------------------------------------------
#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace LoveGame::Script {
    //! @brief ヒロイン1人の定義
    struct CharacterDef {
        std::string id;          // 台本の @show や変数名で使う識別子
        std::string name;        // 台詞の話者として書く表示名
        std::string place;       // カレンダーで会える場所
        std::string prefab;      // ブロックの Prefab
        std::string script;      // 台本ファイル名（Assets/Scripts からの相対）
    };

    //! @brief characters.json の中身
    struct CharacterTable {
        std::vector<CharacterDef> characters;

        const CharacterDef* FindById(const std::string& id) const;
    };

    //! @brief characters.json を読む。失敗したら false
    bool LoadCharacterTable(const std::filesystem::path& path, CharacterTable& out, std::string& error);
}    // namespace LoveGame::Script
