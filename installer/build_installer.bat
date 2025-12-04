@echo off
REM CrossLink-FG Installer Build Script
REM This script builds the Windows Installer Package using WiX Toolset

echo ========================================
echo CrossLink-FG Installer Build Script
echo ========================================
echo.

REM Check if WiX is installed
echo [INFO] Checking WiX Toolset installation...
where candle.exe >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] WiX Toolset is not installed or not in PATH.
    echo Please install WiX Toolset from: https://github.com/wixtoolset/wix3/releases
    echo.
    echo Installation steps:
    echo 1. Download WiX 3.14 or later
    echo 2. Install with default options
    echo 3. Restart your command prompt
    pause
    exit /b 1
)

echo [SUCCESS] WiX Toolset found.

REM Set environment variables
set "WIX_ROOT=%WIX%"
set "INSTALLER_OUTPUT=%~dp0..\output"
set "BUILD_DIR=%~dp0..\build"
set "SOURCE_DIR=%~dp0.."

REM Create output directory
echo [INFO] Creating output directory...
if not exist "%INSTALLER_OUTPUT%" mkdir "%INSTALLER_OUTPUT%"

REM Clean previous builds
echo [INFO] Cleaning previous builds...
if exist "%INSTALLER_OUTPUT%\*.msi" del "%INSTALLER_OUTPUT%\*.msi"
if exist "%INSTALLER_OUTPUT%\*.wixobj" del "%INSTALLER_OUTPUT%\*.wixobj"
if exist "%INSTALLER_OUTPUT%\*.wixpdb" del "%INSTALLER_OUTPUT%\*.wixpdb"

echo.
echo ========================================
echo Step 1: Building Core Components...
echo ========================================

REM Build the main project first
echo [INFO] Building CrossLink-FG project...
cd "%SOURCE_DIR%"
if not exist "build" mkdir "build"
cd build

REM Check if CMake is available
where cmake.exe >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [WARNING] CMake not found. Skipping CMake build.
) else (
    echo [INFO] Running CMake configuration...
    cmake .. -DCMAKE_BUILD_TYPE=Release
    if %ERRORLEVEL% NEQ 0 (
        echo [ERROR] CMake configuration failed.
        pause
        exit /b 1
    )

    echo [INFO] Building project...
    cmake --build . --config Release
    if %ERRORLEVEL% NEQ 0 (
        echo [ERROR] Build failed.
        pause
        exit /b 1
    )
)

cd "%SOURCE_DIR%"

echo.
echo ========================================
echo Step 2: Compiling WiX Source Files...
echo ========================================

REM Compile WiX source files
echo [INFO] Compiling installer source files...

candle.exe -arch x64 -out "%INSTALLER_OUTPUT%\Product.wixobj" "%SOURCE_DIR%\installer\CrossLink-FG.wxs"
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile CrossLink-FG.wxs
    pause
    exit /b 1
)

candle.exe -arch x64 -out "%INSTALLER_OUTPUT%\Components.wixobj" "%SOURCE_DIR%\installer\Components.wxs"
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile Components.wxs
    pause
    exit /b 1
)

candle.exe -arch x64 -out "%INSTALLER_OUTPUT%\Product_core.wixobj" "%SOURCE_DIR%\installer\Product.wxs"
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile Product.wxs
    pause
    exit /b 1
)

echo [SUCCESS] WiX source files compiled.

echo.
echo ========================================
echo Step 3: Linking Installer...
echo ========================================

REM Link the installer
echo [INFO] Linking installer...

light.exe ^
    -out "%INSTALLER_OUTPUT%\CrossLink-FG_Setup.msi" ^
    -cultures:en-US ^
    -ext WixUIExtension ^
    -ext WixUtilExtension ^
    -ext WixFirewallExtension ^
    "%INSTALLER_OUTPUT%\Product.wixobj" ^
    "%INSTALLER_OUTPUT%\Components.wixobj" ^
    "%INSTALLER_OUTPUT%\Product_core.wixobj"

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to link installer
    pause
    exit /b 1
)

echo.
echo ========================================
echo Step 4: Validating Installer...
echo ========================================

REM Validate the installer
echo [INFO] Validating installer...

msival2.exe "%INSTALLER_OUTPUT%\CrossLink-FG_Setup.msi"
if %ERRORLEVEL% NEQ 0 (
    echo [WARNING] Installer validation found issues.
    echo The installer may still work, but review the warnings above.
)

echo.
echo ========================================
echo Build Complete!
echo ========================================
echo.
echo Installer created: %INSTALLER_OUTPUT%\CrossLink-FG_Setup.msi
echo Installer size: 
for %%A in ("%INSTALLER_OUTPUT%\CrossLink-FG_Setup.msi") do echo   %%~zA bytes
echo.
echo [SUCCESS] CrossLink-FG installer build completed successfully!
echo.
echo Next steps:
echo 1. Test the installer on a clean system
echo 2. Sign the installer with your code signing certificate
echo 3. Upload to your distribution platform
echo.
pause
