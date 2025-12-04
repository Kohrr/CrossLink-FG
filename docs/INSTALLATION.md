# CrossLink-FG Installation Guide

**Version:** 1.0.0  
**Last Updated:** December 4, 2025  
**Target Games:** Borderlands 3, Borderlands 4

## Table of Contents

1. [System Requirements](#system-requirements)
2. [Installation Methods](#installation-methods)
3. [Step-by-Step Installation](#step-by-step-installation)
4. [Game Configuration](#game-configuration)
5. [Verification Steps](#verification-steps)
6. [Troubleshooting Installation Issues](#troubleshooting-installation-issues)

## System Requirements

### Minimum Requirements
- **Operating System:** Windows 10 64-bit (Version 1903) or later
- **GPU:** NVIDIA RTX series (RTX 20xx or newer) for primary GPU, AMD RDNA series for secondary GPU
- **VRAM:** 8GB+ per GPU
- **RAM:** 16GB system memory
- **Storage:** 2GB free disk space

### Recommended Requirements
- **Operating System:** Windows 11 64-bit (Latest version)
- **GPU:** NVIDIA RTX 40xx series (primary), AMD RX 7000 series (secondary)
- **VRAM:** 12GB+ per GPU
- **RAM:** 32GB system memory
- **Storage:** 5GB free disk space (SSD recommended)

### Software Dependencies
- DirectX 12 Runtime
- NVIDIA GeForce Experience (latest version)
- AMD Radeon Software (latest version)
- Visual C++ Redistributable 2022 x64
- Windows SDK 10.0.22621 or later

## Installation Methods

### Method 1: Pre-built Installer (Recommended)

1. Download the latest installer from the [official releases](https://github.com/crosslink-fg/releases)
2. Run the installer as Administrator
3. Follow the installation wizard

### Method 2: Build from Source

See the [Developer Guide](DEVELOPER_GUIDE.md) for detailed build instructions.

## Step-by-Step Installation

### Prerequisites Check

1. **Update Graphics Drivers:**
   ```bash
   # NVIDIA
   # Download latest drivers from nvidia.com
   
   # AMD
   # Download latest drivers from amd.com
   ```

2. **Enable Hardware Acceleration:**
   - Windows Settings → Gaming → Xbox Game Bar → Turn off
   - Control Panel → Hardware and Sound → Power Options → High Performance

3. **Check DirectX Version:**
   - Press `Win + R`, type `dxdiag`, press Enter
   - Navigate to "System" tab
   - Verify DirectX 12 or later is installed

### Installation Process

1. **Run Installer as Administrator**
   - Right-click on installer → "Run as administrator"
   - Accept the End User License Agreement (EULA)

2. **Choose Installation Location**
   - Default: `C:\Program Files\CrossLink-FG\`
   - Ensure write permissions to chosen location

3. **Select Components**
   - Core Framework ✓
   - Game Support (Borderlands 3/4) ✓
   - Development Tools (optional)
   - Source Code (developers only)

4. **Installation Progress**
   - Monitor progress bar
   - Wait for completion message

5. **Post-Installation Setup**
   - Restart system (recommended)
   - Run initial configuration

### Game Configuration

#### Borderlands 3

1. **Navigate to Game Directory:**
   ```
   C:\Program Files (x86)\Steam\steamapps\common\Borderlands 3\OakGame\Binaries\Win64\
   ```

2. **Create Backup of Original Files:**
   ```bash
   copy version.dll version.dll.backup
   ```

3. **Copy CrossLink-FG Proxy:**
   ```bash
   copy "C:\Program Files\CrossLink-FG\games\Borderlands3\version.dll" .
   ```

4. **Game Launch Parameters:**
   ```
   -dx12 -windowed -maximized
   ```

#### Borderlands 4

1. **Navigate to Game Directory:**
   ```
   C:\Program Files (x86)\Steam\steamapps\common\Borderlands 4\Game\Binaries\Win64\
   ```

2. **Apply Similar Steps as Borderlands 3**

### Configuration File Setup

1. **Location:**
   ```
   %APPDATA%\CrossLink-FG\config.ini
   ```

2. **Basic Configuration:**
   ```ini
   [General]
   EnableFrameGeneration=1
   FrameGenerationQuality=High
   TargetFrameRate=60
   OverlayEnabled=1
   
   [Graphics]
   PrimaryGPU=0
   SecondaryGPU=1
   UseNvidiaOpticalFlow=1
   OpenCLDeviceId=0
   
   [Performance]
   AsyncProcessing=1
   ThreadPriority=High
   MemoryPoolSize=2048
   ```

## Verification Steps

### 1. System Check
Run the built-in system check:
```bash
cd "C:\Program Files\CrossLink-FG\"
CrossLink-FG.exe --check-system
```

### 2. Game Test
1. Launch Borderlands 3/4
2. Look for overlay in top-left corner
3. Check for performance improvement
4. Verify no crashes or artifacts

### 3. Performance Verification
Monitor GPU usage and frame times using:
- NVIDIA-SMI
- GPU-Z
- MSI Afterburner

## Troubleshooting Installation Issues

### Common Issues

#### Issue: "DirectX 12 not found"
**Solution:**
1. Install latest DirectX Runtime
2. Verify Windows 10/11 is up to date
3. Reinstall graphics drivers

#### Issue: "Invalid GPU configuration"
**Solution:**
1. Ensure both GPUs are properly detected
2. Update graphics drivers
3. Check GPU compatibility

#### Issue: "Game not launching with overlay"
**Solution:**
1. Verify proxy DLL is in game directory
2. Check game executable permissions
3. Run game as administrator

#### Issue: "Performance degradation"
**Solution:**
1. Verify both GPUs are active
2. Check thermal throttling
3. Adjust quality settings

#### Issue: "Overlay not visible"
**Solution:**
1. Check overlay settings in config.ini
2. Verify game DirectX 12 mode
3. Restart game

### Log Files

Location: `%APPDATA%\CrossLink-FG\logs\`

- `installation.log` - Installation process
- `runtime.log` - Runtime execution
- `error.log` - Error messages

### Support

For additional support:
- Documentation: [User Guide](USER_GUIDE.md)
- API Reference: [API_REFERENCE.md](API_REFERENCE.md)
- GitHub Issues: [Report bugs](https://github.com/crosslink-fg/issues)
- Email: support@crosslink-fg.github.io

### Uninstallation

1. **Using Control Panel:**
   - Programs and Features
   - Find "CrossLink-FG"
   - Click Uninstall

2. **Manual Removal:**
   ```bash
   # Remove installation directory
   rmdir /s "C:\Program Files\CrossLink-FG\"
   
   # Remove configuration
   rmdir /s "%APPDATA%\CrossLink-FG\"
   
   # Remove game proxies
   del "C:\Program Files (x86)\Steam\steamapps\common\Borderlands 3\OakGame\Binaries\Win64\version.dll"
   ```

3. **Restore Original Game Files:**
   - Restore `version.dll.backup` if available

### Version History

- **v1.0.0** (December 4, 2025)
  - Initial release
  - Borderlands 3/4 support
  - ImGui overlay system
  - Cross-GPU frame generation

For more detailed information, see the [User Guide](USER_GUIDE.md) and [Developer Guide](DEVELOPER_GUIDE.md).
