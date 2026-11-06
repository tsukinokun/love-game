@echo off
rem ---------------------------------------------------------------------------
rem LoveGame build.   build.bat [Debug|Release]
rem Prints nothing on success, error lines only on failure.
rem Regenerates the solution every run so new source files are picked up.
rem ---------------------------------------------------------------------------
setlocal

set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Debug"

set "ROOT=%~dp0"
set "SLN=%ROOT%.build\LoveGame.sln"

set "MSBUILD="
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" call :find_vs2022
if not defined MSBUILD if exist "%VSWHERE%" call :find_latest
if not defined MSBUILD set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"

if not exist "%MSBUILD%" (
    echo build.bat: MSBuild not found. Install Visual Studio 2022 with the "Desktop development with C++" workload.
    exit /b 1
)

pushd "%ROOT%"
"%ROOT%External\TsukinoEngine\vendor\premake5.exe" vs2022 >nul 2>nul
set "PREMAKE_RESULT=%ERRORLEVEL%"
popd
if not "%PREMAKE_RESULT%"=="0" exit /b 1

rem premake emits packages.config, so -t:restore alone does not restore it
if not exist "%ROOT%.build\packages" (
    "%MSBUILD%" "%SLN%" -t:restore -p:RestorePackagesConfig=true -p:Configuration=%CONFIG% -p:Platform=x64 -nologo -v:q -clp:"ErrorsOnly;NoSummary"
    if errorlevel 1 exit /b 1
)

"%MSBUILD%" "%SLN%" /p:Configuration=%CONFIG% /p:Platform=x64 /m /nologo /v:q /clp:"ErrorsOnly;NoSummary"
exit /b %ERRORLEVEL%

:find_vs2022
for /f "usebackq tokens=*" %%i in (`^""%VSWHERE%" -version "[17.0,18.0)" -products * -requires Microsoft.Component.MSBuild -property installationPath^"`) do (
    if exist "%%i\MSBuild\Current\Bin\MSBuild.exe" set "MSBUILD=%%i\MSBuild\Current\Bin\MSBuild.exe"
)
goto :eof

:find_latest
for /f "usebackq tokens=*" %%i in (`^""%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath^"`) do (
    if exist "%%i\MSBuild\Current\Bin\MSBuild.exe" set "MSBUILD=%%i\MSBuild\Current\Bin\MSBuild.exe"
)
goto :eof
