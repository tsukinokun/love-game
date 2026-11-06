//--------------------------------------------------------------
//! @file   SaveSystem.hpp
//! @brief  セーブデータの読み書き
//--------------------------------------------------------------
#pragma once
#include <Game/Game.hpp>

namespace LoveGame::SaveSystem {
    //! @brief スロットへ保存する。途中で落ちても前のデータは壊れない
    //! @param slot  [in] 0 始まりのスロット番号
    //! @param state [in] 保存する状態
    //! @return 保存できたら true
    bool Save(int slot, const GameState& state);

    //! @brief スロットから読む
    //! @param slot  [in]  0 始まりのスロット番号
    //! @param state [out] 読み込んだ状態。失敗したら変更しない
    //! @return 読めたら true
    bool Load(int slot, GameState& state);
}    // namespace LoveGame::SaveSystem
