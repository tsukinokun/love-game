//--------------------------------------------------------------
//! @file   ButtonList.hpp
//! @brief  縦に並ぶボタン。個数が変わるので、雛形の Prefab から作る
//--------------------------------------------------------------
#pragma once
#include <Script/GameConfig.hpp>

#include <Tsukino/Engine/ECS/Prefab/PrefabFactory.hpp>
#include <Tsukino/Engine/ECS/Scene.hpp>

#include <string>
#include <vector>

namespace LoveGame::UI {
    class ButtonList {
    public:
        //! @brief ボタンを並べる。前のボタンは消す
        //! @param scene   [in] 置くシーン
        //! @param factory [in] Prefab の生成元
        //! @param prefab  [in] ボタンの雛形
        //! @param labels  [in] ボタンの文字
        //! @param config  [in] 並びの設定
        //! @param centerX [in] 並びの中心の X 座標
        //! @param centerY [in] 並びの中心の Y 座標
        void Show(Tsukino::ECS::Scene& scene, Tsukino::Engine::ECS::Prefab::PrefabFactory& factory, const std::string& prefab,
                  const std::vector<std::wstring>& labels, const Script::UiConfig& config, float centerX, float centerY);

        //! @brief ボタンをすべて消す
        void Clear();

        //! @brief このフレームに押されたボタンの番号。無ければ -1
        int ClickedIndex() const;

        bool IsOpen() const { return !m_buttons.empty(); }

    private:
        Tsukino::ECS::Scene*              m_scene = nullptr;
        std::vector<Tsukino::ECS::Entity> m_buttons;
    };
}    // namespace LoveGame::UI
