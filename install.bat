@echo off
setlocal

REM Usage: install.bat <module_name> <target_path>
REM Example: install.bat core "C:\project\x64\Release\core.lib"

set MODULE=%~1
set TARGET_PATH=%~2

if "%MODULE%"=="" (
    echo Error: Module name not specified
    exit /b 1
)

if "%TARGET_PATH%"=="" (
    echo Error: Target path not specified
    exit /b 1
)

set DIST_DIR=%~dp0dist
set INCLUDE_DIST=%DIST_DIR%\include\ducklib
set LIB_DIST=%DIST_DIR%\lib

if not exist "%INCLUDE_DIST%\ducklib\%MODULE%" mkdir "%INCLUDE_DIST%\ducklib\%MODULE%"
if not exist "%LIB_DIST%" mkdir "%LIB_DIST%"

xcopy /Y /S /I "%~dp0%MODULE%\include\ducklib\%MODULE%\*.h" "%INCLUDE_DIST%\%MODULE%\"

xcopy /Y "%TARGET_PATH%" "%LIB_DIST%\"

echo [%MODULE%] Installed to dist