@echo off
echo === CrossLink-FG Build Verification Script ===
echo Verifying version.dll build and timestamp
echo.

REM Check if version.dll exists
if not exist "build\Release\version.dll" (
    echo [ERROR] version.dll not found in build\Release\
    echo Run build process first
    pause
    exit /b 1
)

REM Show detailed file information
echo [INFO] version.dll details:
dir /b /s "build\Release\version.dll" | find "version.dll"
echo.
echo Timestamp and properties:
dir "build\Release\version.dll" | find "version.dll"
echo.

REM Show file size
for %%A in ("build\Release\version.dll") do (
    echo File size:          %%~zA bytes
    echo Last modified:      %%~tA
    echo File attributes:    %%~aA
)

echo.
echo === SIZE VERIFICATION ===
echo Expected size: Around 290KB (292,352 bytes)
for %%A in ("build\Release\version.dll") do echo Actual size: %%~zA bytes

echo.
echo === TIMESTAMP VERIFICATION ===
echo If timestamp is 19:00 Uhr - build is from old compilation
echo If timestamp is 20:xx Uhr - build includes latest fixes
echo.

echo Verify these critical fixes are included:
echo [ ] CMakeLists.txt: ImGui static library, dx12_hooks integration
echo [ ] proxy.cpp: Windows DLL function forwarding complete
echo [ ] dx12_hooks.cpp: ExecuteCommandLists hook implemented
echo.

pause
