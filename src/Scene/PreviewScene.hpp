//--------------------------------------------------------------
//! @file   PreviewScene.hpp
//! @brief  取り込んだモデルの見た目を確認するシーン。LoveGame.exe preview で起動する
//--------------------------------------------------------------
#pragma once
#include <Scene/GameScene.hpp>

namespace LoveGame {
    class PreviewScene : public GameScene {
    public:
        using GameScene::GameScene;

    protected:
        void Setup() override;
        void Tick(float deltaTime) override;
    };
}    // namespace LoveGame
