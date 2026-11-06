----------------------------------------
-- LoveGame のビルド定義
-- 生成: vendor の premake を open.bat / build.bat 経由で実行する
----------------------------------------
include "External/TsukinoEngine/Tools/premake/tsukino.lua"

workspace "LoveGame"
    startproject "LoveGame"
    location ".build"
    tsukino_workspace_defaults()

include "External/TsukinoEngine"

----------------------------------------
-- 台本エンジン。描画を知らない静的ライブラリにしておくことで、
-- 検証ツールとゲーム本体が同じパーサを共有できる
----------------------------------------
project "LoveGame.Script"
    location ".build/LoveGame.Script"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetdir ("bin/%{cfg.buildcfg}")
    objdir ("bin-int/%{cfg.buildcfg}/%{prj.name}")
    files { "src/Script/**.hpp", "src/Script/**.cpp" }
    includedirs { "src", "External/TsukinoEngine/External/cereal/include" }

----------------------------------------
-- 台本の検証と行ID付与を行うコンソールツール
----------------------------------------
project "LoveGame.ScriptCheck"
    location ".build/LoveGame.ScriptCheck"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    targetdir ("bin/%{cfg.buildcfg}")
    objdir ("bin-int/%{cfg.buildcfg}/%{prj.name}")
    debugdir "."
    files { "Tools/ScriptCheck/**.cpp" }
    includedirs { "src", "External/TsukinoEngine/External/cereal/include" }
    links { "LoveGame.Script" }

----------------------------------------
-- ゲーム本体
----------------------------------------
project "LoveGame"
    location ".build/LoveGame"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++20"
    targetdir ("bin/%{cfg.buildcfg}")
    objdir ("bin-int/%{cfg.buildcfg}/%{prj.name}")

    filter "action:vs*"
        buildoptions { "/permissive-" }
    filter {}

    files { "src/**.hpp", "src/**.cpp" }
    removefiles { "src/Script/**" }
    includedirs { "src" }

    links { "LoveGame.Script" }
    tsukino_link()
    tsukino_release_payload()

    -- Debug はリポジトリルートを実行時の基準にするためコピー不要。
    -- Release は exe の隣へ Assets を同期する
    filter "configurations:Release"
        postbuildcommands(tsukino_mirror_commands("Assets", "Assets"))
    filter {}
