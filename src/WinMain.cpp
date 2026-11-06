//--------------------------------------------------------------
//! @file   WinMain.cpp
//! @brief  LoveGame のエントリポイント
//--------------------------------------------------------------
#include <Game/Game.hpp>
#include <Scene/MenuScenes.hpp>
#include <Scene/PreviewScene.hpp>
#include <Scene/StageScene.hpp>
#include <UI/UITextComponent.hpp>

#include <Tsukino/BuiltIn/ECS/Component/PointerTargetComponent.hpp>
#include <Tsukino/Core/Log.hpp>
#include <Tsukino/Engine/ECS/Prefab/PrefabFactory.hpp>
#include <Tsukino/EngineIntegration/EngineAPI.hpp>
#include <Tsukino/EngineIntegration/EngineIntegration.hpp>

#include <Windows.h>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <memory>

namespace {
    template <class SceneType>
    void Register(Tsukino::EngineIntegration::EngineAPI& api, const char* name, LoveGame::Game& game) {
        api.RegisterScene(name, [&game]() -> std::unique_ptr<Tsukino::EngineIntegration::GameSceneBase> {
            return std::make_unique<SceneType>(game);
        });
    }
}    // namespace

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR lpCmdLine, _In_ int) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // これを呼ばないと、デバッガの外ではエンジンの警告が一切見えない
    Tsukino::Core::Log::SetLogFile("Logs/LoveGame.log");

    LoveGame::Game game;
    std::string    error;
    if(!game.LoadData(error)) {
        Tsukino::Core::Log::Error(error);
        return EXIT_FAILURE;
    }

    Tsukino::EngineIntegration::EngineIntegration engine;
    if(!engine.Initialize(1600, 900, "LoveGame")) {
        Tsukino::Core::Log::Error("Failed to initialize EngineIntegration.");
        return EXIT_FAILURE;
    }

    // ゲーム側で Prefab に書けるようにするコンポーネント
    engine.GetContext().prefabFactory->RegisterComponent<LoveGame::UI::UITextComponent>("UITextComponent");
    engine.GetContext().prefabFactory->RegisterComponent<Tsukino::BuiltIn::ECS::PointerTargetComponent>("PointerTargetComponent");

    Tsukino::EngineIntegration::EngineAPI api(engine.GetContext());
    Register<LoveGame::TitleScene>(api, LoveGame::SceneName::kTitle, game);
    Register<LoveGame::NameEntryScene>(api, LoveGame::SceneName::kNameEntry, game);
    Register<LoveGame::CalendarScene>(api, LoveGame::SceneName::kCalendar, game);
    Register<LoveGame::StageScene>(api, LoveGame::SceneName::kStage, game);
    Register<LoveGame::PreviewScene>(api, LoveGame::SceneName::kPreview, game);

    // LoveGame.exe preview [場面番号] でモデル確認シーンから始める
    const std::string commandLine = lpCmdLine != nullptr ? lpCmdLine : "";
    if(commandLine.find("preview") != std::string::npos && !game.preview.shots.empty()) {
        const std::size_t digit = commandLine.find_last_of("123456789");
        if(digit != std::string::npos) {
            const int index = commandLine[digit] - '1';
            if(index < static_cast<int>(game.preview.shots.size())) game.previewIndex = index;
        }
        api.ChangeScene(std::string(LoveGame::SceneName::kPreview));
    } else {
        api.ChangeScene(std::string(LoveGame::SceneName::kTitle));
    }

    auto lastTime = std::chrono::steady_clock::now();
    while(api.ProcessMessages()) {
        const auto now       = std::chrono::steady_clock::now();
        float      deltaTime = std::chrono::duration<float>(now - lastTime).count();
        lastTime             = now;
        deltaTime            = std::min(deltaTime, 1.0f / 15.0f);

        api.Update(deltaTime);
        api.Render();
    }
    return EXIT_SUCCESS;
}
