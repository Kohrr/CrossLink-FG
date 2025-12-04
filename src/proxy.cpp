// Proxy DLL implementation for version.dll
// This DLL acts as a "Trojan Horse" - the game loads it as "version.dll"
// but it also injects our CrossLink FG functionality

#pragma comment(linker,"/export:GetFileVersionInfoA=C:\\Windows\\System32\\version.dll.GetFileVersionInfoA")
#pragma comment(linker,"/export:GetFileVersionInfoByHandle=C:\\Windows\\System32\\version.dll.GetFileVersionInfoByHandle")
#pragma comment(linker,"/export:GetFileVersionInfoExA=C:\\Windows\\System32\\version.dll.GetFileVersionInfoExA")
#pragma comment(linker,"/export:GetFileVersionInfoExW=C:\\Windows\\System32\\version.dll.GetFileVersionInfoExW")
#pragma comment(linker,"/export:GetFileVersionInfoSizeA=C:\\Windows\\System32\\version.dll.GetFileVersionInfoSizeA")
#pragma comment(linker,"/export:GetFileVersionInfoSizeExA=C:\\Windows\\System32\\version.dll.GetFileVersionInfoSizeExA")
#pragma comment(linker,"/export:GetFileVersionInfoSizeExW=C:\\Windows\\System32\\version.dll.GetFileVersionInfoSizeExW")
#pragma comment(linker,"/export:GetFileVersionInfoSizeW=C:\\Windows\\System32\\version.dll.GetFileVersionInfoSizeW")
#pragma comment(linker,"/export:GetFileVersionInfoW=C:\\Windows\\System32\\version.dll.GetFileVersionInfoW")
#pragma comment(linker,"/export:VerFindFileA=C:\\Windows\\System32\\version.dll.VerFindFileA")
#pragma comment(linker,"/export:VerFindFileW=C:\\Windows\\System32\\version.dll.VerFindFileW")
#pragma comment(linker,"/export:VerInstallFileA=C:\\Windows\\System32\\version.dll.VerInstallFileA")
#pragma comment(linker,"/export:VerInstallFileW=C:\\Windows\\System32\\version.dll.VerInstallFileW")
#pragma comment(linker,"/export:VerLanguageNameA=C:\\Windows\\System32\\version.dll.VerLanguageNameA")
#pragma comment(linker,"/export:VerLanguageNameW=C:\\Windows\\System32\\version.dll.VerLanguageNameW")
#pragma comment(linker,"/export:VerQueryValueA=C:\\Windows\\System32\\version.dll.VerQueryValueA")
#pragma comment(linker,"/export:VerQueryValueW=C:\\Windows\\System32\\version.dll.VerQueryValueW")

// Note: This file only provides the forwarding exports.
// The actual logic is handled in dllmain.cpp in the MainThread function.
