# CrossLink-FG Troubleshooting Guide

**Version:** 1.0.0  
**Last Updated:** December 4, 2025  
**Support Level:** Comprehensive Troubleshooting Reference

## Table of Contents

1. [Quick Diagnostic Checklist](#quick-diagnostic-checklist)
2. [System Requirements Issues](#system-requirements-issues)
3. [Installation Problems](#installation-problems)
4. [GPU and Driver Issues](#gpu-and-driver-issues)
5. [DirectX 12 Problems](#directx-12-problems)
6. [Game Integration Issues](#game-integration-issues)
7. [Performance Problems](#performance-problems)
8. [ImGui Overlay Issues](#imgui-overlay-issues)
9. [Frame Generation Failures](#frame-generation-failures)
10. [NVIDIA Optical Flow Issues](#nvidia-optical-flow-issues)
11. [OpenCL Processing Issues](#opencl-processing-issues)
12. [Memory and Resource Problems](#memory-and-resource-problems)
13. [Log File Analysis](#log-file-analysis)
14. [Advanced Diagnostics](#advanced-diagnostics)
15. [Support and Reporting](#support-and-reporting)

## Quick Diagnostic Checklist

Before diving into specific issues, run through this checklist:

### System Check
- [ ] **Operating System:** Windows 10 64-bit (1903+) or Windows 11
- [ ] **Administrator Rights:** Running with elevated privileges
- [ ] **Antivirus:** Temporarily disabled for testing
- [ ] **User Account Control:** UAC disabled or running as admin
- [ ] **Windows Updates:** System fully updated
- [ ] **DirectX 12:** Verified in dxdiag tool

### Hardware Check
- [ ] **GPU Requirements:** NVIDIA RTX + AMD RDNA configured
- [ ] **GPU Drivers:** Latest versions from NVIDIA/AMD
- [ ] **Power Supply:** Adequate wattage for dual-GPU setup
- [ ] **Cooling:** GPU temperatures under 80°C during operation
- [ ] **Memory:** Sufficient system RAM (16GB+)

### Software Check
- [ ] **CrossLink-FG:** Latest version installed
- [ ] **Game Compatibility:** Supported DirectX 12 game detected
- [ ] **Overlay Settings:** ImGui overlay enabled and visible
- [ ] **Configuration:** Valid settings in config files
- [ ] **Log Files:** Check for error messages

### Quick Test Commands
```cmd
# Run system check
cd "C:\Program Files\CrossLink-FG\"
CrossLink-FG.exe --check-system

# Run performance test
CrossLink-FG.exe --test-performance

# Check logs
notepad %APPDATA%\CrossLink-FG\logs\runtime.log
```

## System Requirements Issues

### Issue: "Unsupported Operating System"

**Symptoms:**
- Error message on startup
- Incompatible OS message
- System check fails

**Solutions:**

1. **Check OS Version**
   ```cmd
   winver
   ```
   - Minimum: Windows 10 version 1903 (Build 18362)
   - Recommended: Windows 11 latest version

2. **Update Windows**
   - Windows Update → Check for updates
   - Install all cumulative updates
   - Restart system

3. **System Requirements Check**
   ```powershell
   # Run PowerShell as Administrator
   Get-ComputerInfo | Select-Object WindowsProductName, WindowsVersion, WindowsBuildLabEx
   ```

### Issue: "Insufficient System Memory"

**Symptoms:**
- Out of memory errors
- System instability
- Poor performance

**Solutions:**

1. **Check Available Memory**
   ```cmd
   wmic computersystem get TotalPhysicalMemory
   ```

2. **Increase Virtual Memory**
   - Control Panel → System → Advanced System Settings
   - Performance Settings → Advanced → Virtual Memory
   - Set custom size to 2x physical RAM

3. **Close Unnecessary Applications**
   - Task Manager → End non-essential processes
   - Reduce memory pool size in CrossLink-FG settings

### Issue: "DirectX 12 Not Available"

**Symptoms:**
- DirectX initialization failed
- Game won't start with CrossLink-FG
- DX12 feature missing error

**Solutions:**

1. **Verify DirectX Installation**
   ```cmd
   dxdiag
   ```
   Check "System" tab for DirectX version

2. **Install DirectX Runtime**
   - Download latest DirectX End-User Runtime
   - Install and restart system

3. **Enable Hardware Acceleration**
   - Windows Settings → Gaming → Xbox Game Bar → Off
   - Control Panel → Power Options → High Performance

## Installation Problems

### Issue: "Installer Failed to Start"

**Symptoms:**
- MSI installation error
- Setup.exe crashes
- Access denied message

**Solutions:**

1. **Run as Administrator**
   - Right-click installer → "Run as administrator"
   - Temporarily disable UAC
   - Use elevated command prompt

2. **Check File Permissions**
   ```cmd
   # Check installation directory
   icacls "C:\Program Files\CrossLink-FG"
   
   # Fix permissions if needed
   icacls "C:\Program Files\CrossLink-FG" /grant Users:F /t
   ```

3. **Disable Antivirus**
   - Temporarily disable real-time protection
   - Add installer to antivirus exceptions
   - Whitelist installation directory

4. **Clean Installation**
   ```cmd
   # Remove previous installation
   rmdir /s /q "C:\Program Files\CrossLink-FG"
   rmdir /s /q "%APPDATA%\CrossLink-FG"
   
   # Remove registry entries (if applicable)
   reg delete "HKEY_LOCAL_MACHINE\SOFTWARE\CrossLink-FG" /f
   ```

### Issue: "Installation Completes but Files Missing"

**Symptoms:**
- Program runs but features missing
- Missing DLL files
- Configuration errors

**Solutions:**

1. **Repair Installation**
   - Control Panel → Programs → CrossLink-FG → Repair

2. **Manual File Verification**
   ```cmd
   # Check critical files exist
   dir "C:\Program Files\CrossLink-FG\bin"
   dir "C:\Program Files\CrossLink-FG\games"
   ```

3. **Reinstall with Full Features**
   - Run installer again
   - Select all components
   - Choose "Repair" option

## GPU and Driver Issues

### Issue: "GPU Not Detected"

**Symptoms:**
- Only one GPU detected
- "No compatible GPU found" error
- Device manager shows unknown devices

**Solutions:**

1. **Update GPU Drivers**
   ```cmd
   # NVIDIA
   # Download from: nvidia.com/drivers
   
   # AMD
   # Download from: amd.com/support
   ```

2. **Check GPU in Device Manager**
   - Win+X → Device Manager
   - Expand "Display adapters"
   - Verify both GPUs listed without errors

3. **Enable GPUs in BIOS**
   - Restart system
   - Enter BIOS/UEFI setup
   - Enable both GPUs if available
   - Set PCIe x16 mode for primary GPU

4. **Power Management**
   - NVIDIA Control Panel → Manage 3D Settings
   - Set "Power Management Mode" to "Prefer Maximum Performance"

### Issue: "GPU Temperature Too High"

**Symptoms:**
- Performance degradation
- System throttling
- Thermal shutdown warnings

**Solutions:**

1. **Monitor GPU Temperatures**
   ```cmd
   # Using NVIDIA-SMI
   nvidia-smi
   
   # Using GPU-Z (download separately)
   ```

2. **Improve Cooling**
   - Clean GPU fans and heatsinks
   - Improve case airflow
   - Underclock if necessary
   - Adjust fan curves

3. **Reduce Load**
   - Lower frame generation quality
   - Reduce target FPS
   - Close background applications

### Issue: "Insufficient VRAM"

**Symptoms:**
- Memory allocation failures
- Poor frame generation quality
- System crashes

**Solutions:**

1. **Check VRAM Usage**
   ```cmd
   # NVIDIA
   nvidia-smi -q -d MEMORY
   
   # AMD (Radeon Software)
   ```

2. **Reduce Memory Pool**
   - CrossLink-FG Settings → Performance
   - Reduce "Memory Pool Size"
   - Start with 1024MB and increase gradually

3. **Close Memory-Intensive Apps**
   - Browser tabs
   - Video editing software
   - Multiple games

## DirectX 12 Problems

### Issue: "Game Not Using DirectX 12"

**Symptoms:**
- CrossLink-FG doesn't activate
- Game runs in DirectX 11 mode
- "DX12 required" message

**Solutions:**

1. **Check Game Settings**
   - Borderlands 3: Settings → Graphics → API → DirectX 12
   - Borderlands 4: Game options → Rendering → DirectX 12

2. **Launch Game with DX12**
   ```
   Borderlands3.exe -dx12
   ```

3. **Verify System DirectX 12 Support**
   ```cmd
   dxdiag → System tab → DirectX version
   ```

### Issue: "DirectX 12 Hooking Failed"

**Symptoms:**
- Hook installation errors
- Game crashes on startup
- Overlay doesn't appear

**Solutions:**

1. **Run as Administrator**
   - Always launch both game and CrossLink-FG as admin

2. **Check for Conflicts**
   - Disable other overlay software (MSI Afterburner, RTSS)
   - Remove competing DLL injections

3. **Update DirectX Components**
   - Install latest DirectX Runtime
   - Update Windows to latest version

4. **Registry Check**
   ```cmd
   # Check for corrupted DirectX entries
   reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\DirectX"
   ```

## Game Integration Issues

### Issue: "Game Not Detected"

**Symptoms:**
- CrossLink-FG doesn't recognize game
- No automatic attachment
- Manual configuration required

**Solutions:**

1. **Verify Game Installation**
   ```cmd
   # Check Borderlands 3
   dir "C:\Program Files (x86)\Steam\steamapps\common\Borderlands 3\"
   
   # Check Borderlands 4
   dir "C:\Program Files (x86)\Steam\steamapps\common\Borderlands 4\"
   ```

2. **Manual Game Registration**
   ```cmd
   # Add game to supported list
   CrossLink-FG.exe --add-game "Borderlands3" "path\to\game.exe"
   ```

3. **Check Game Executable**
   - Verify game runs without CrossLink-FG first
   - Check for game updates

### Issue: "Game Crashes with CrossLink-FG"

**Symptoms:**
- Immediate crash on game launch
- Black screen or freeze
- Application error messages

**Solutions:**

1. **Remove and Test**
   ```cmd
   # Remove CrossLink-FG DLL temporarily
   move "path\to\game\version.dll" "path\to\game\version.dll.disabled"
   
   # Test if game runs without CrossLink-FG
   ```

2. **Debug Mode Testing**
   ```cmd
   # Run in debug mode
   CrossLink-FG.exe --debug --game "Borderlands3"
   ```

3. **Safe Mode Configuration**
   - Disable all optional features
   - Use lowest quality settings
   - Minimal memory pool

### Issue: "Proxy DLL Conflicts"

**Symptoms:**
- Game won't start
- "Version.dll not found" error
- Multiple version.dll files

**Solutions:**

1. **Clean Game Directory**
   ```cmd
   cd "path\to\game\"
   
   # Remove all CrossLink-FG files
   del version.dll
   del version.dll.backup
   
   # Restore original
   copy version.dll.original version.dll
   ```

2. **Single Instance Rule**
   - Only one version.dll in game directory
   - Remove other overlay/mods DLLs
   - Check for conflicting versions

## Performance Problems

### Issue: "Performance Worse with Frame Generation"

**Symptoms:**
- Lower FPS than native rendering
- High latency
- Stuttering

**Solutions:**

1. **Quality Settings**
   - Reduce frame generation quality
   - Lower target FPS
   - Disable complex features

2. **System Optimization**
   ```cmd
   # Set high performance power plan
   powercfg -setactive 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c
   
   # Disable Windows Game Mode
   reg add "HKEY_CURRENT_USER\Software\Microsoft\GameBar" /v AutoGameModeEnabled /t REG_DWORD /d 0 /f
   ```

3. **GPU Optimization**
   - Set NVIDIA GPU to Maximum Performance
   - Disable GPU monitoring overlays
   - Close GPU-intensive background apps

### Issue: "High CPU Usage"

**Symptoms:**
- CPU at 100%
- System lag
- Input delay

**Solutions:**

1. **Thread Priority**
   - CrossLink-FG Settings → Performance
   - Set "Thread Priority" to "Normal" or "Below Normal"

2. **Async Processing**
   - Enable "Async Processing" for lower CPU impact
   - Disable if causing instability

3. **Background Processes**
   ```cmd
   # Check CPU usage
   tasklist | findstr CrossLink-FG
   
   # End unnecessary processes
   taskkill /f /im chrome.exe
   ```

### Issue: "Memory Leaks"

**Symptoms:**
- Gradually increasing memory usage
- System instability over time
- Eventually crashes

**Solutions:**

1. **Monitor Memory Usage**
   ```cmd
   # Task Manager → Performance → Memory
   # Check CrossLink-FG.exe memory usage
   
   # Check GPU memory
   nvidia-smi
   ```

2. **Regular Restarts**
   - Restart CrossLink-FG after extended gaming sessions
   - Restart game if memory usage is excessive

3. **Configuration Adjustment**
   - Reduce memory pool size
   - Lower frame resolution if applicable

## ImGui Overlay Issues

### Issue: "Overlay Not Visible"

**Symptoms:**
- No overlay in game
- CrossLink-FG running but invisible

**Solutions:**

1. **Enable Overlay**
   ```cmd
   # Check configuration
   CrossLink-FG.exe --config show-overlay
   
   # Toggle overlay visibility
   CrossLink-FG.exe --overlay on
   ```

2. **Z-Order Issues**
   - Alt+Tab out and back to game
   - Set game to fullscreen mode
   - Disable other overlays

3. **DirectX Context**
   - Verify game runs in DirectX 12
   - Check overlay rendering context

### Issue: "Overlay Frozen or Laggy"

**Symptoms:**
- Overlay updates slowly
- Mouse input lag
- Interface unresponsive

**Solutions:**

1. **Update Rate**
   - Reduce overlay update frequency
   - Disable real-time statistics

2. **Rendering Optimization**
   - Simplify overlay layout
   - Reduce number of displayed metrics

3. **Resource Competition**
   - Disable overlay when not needed
   - Close other monitoring software

## Frame Generation Failures

### Issue: "Frame Generation Not Working"

**Symptoms:**
- No intermediate frames generated
- Same FPS as native rendering
- Frame generation inactive

**Solutions:**

1. **Enable Frame Generation**
   - CrossLink-FG Settings → General
   - Check "Enable Frame Generation"
   - Verify quality setting isn't "Off"

2. **Frame Rate Targets**
   - Set target FPS higher than native
   - Check frame generation ratio in overlay

3. **Processing Pipeline**
   ```cmd
   # Check processing status
   CrossLink-FG.exe --status
   
   # Restart frame generation
   CrossLink-FG.exe --restart-frame-gen
   ```

### Issue: "Poor Frame Generation Quality"

**Symptoms:**
- Visible artifacts
- Ghosting effects
- Blurry interpolated frames

**Solutions:**

1. **Quality Settings**
   - Increase quality level to "High" or "Ultra"
   - Enable temporal filtering
   - Adjust motion smoothing

2. **Motion Estimation**
   - Enable NVIDIA Optical Flow hardware
   - Check motion vector quality

3. **Game-Specific Settings**
   - Disable motion blur in games
   - Reduce motion blur effects
   - Enable high refresh rate mode

## NVIDIA Optical Flow Issues

### Issue: "Optical Flow Initialization Failed"

**Symptoms:**
- "Hardware optical flow unavailable"
- Falls back to software mode
- Reduced performance

**Solutions:**

1. **Hardware Compatibility**
   - Verify RTX 20xx+ GPU
   - Update to latest NVIDIA drivers
   - Enable hardware features in NVIDIA Control Panel

2. **Feature Support**
   ```cmd
   # Check optical flow support
   nvidia-smi -q -d CAPABILITIES
   ```

3. **Registry Settings**
   ```cmd
   # Enable optical flow features
   reg add "HKEY_LOCAL_MACHINE\SOFTWARE\NVIDIA Corporation\Global\NGX" /v EnableNgxFeatures /t REG_DWORD /d 1 /f
   ```

### Issue: "Optical Flow Performance Poor"

**Symptoms:**
- Optical flow slower than expected
- High CPU usage during analysis
- Frame processing delays

**Solutions:**

1. **Hardware Limitations**
   - Check GPU thermal throttling
   - Verify adequate power supply
   - Monitor GPU utilization

2. **Resolution Scaling**
   - Reduce optical flow resolution
   - Use adaptive quality based on scene complexity

## OpenCL Processing Issues

### Issue: "OpenCL Initialization Failed"

**Symptoms:**
- "No OpenCL platforms found"
- CPU fallback only
- Processing errors

**Solutions:**

1. **GPU Drivers**
   - Update AMD drivers
   - Install OpenCL Runtime from AMD
   - Verify OpenCL in device manager

2. **OpenCL Runtime**
   ```cmd
   # Check OpenCL platforms
   # Use tools like GPU Caps Viewer
   
   # Reinstall OpenCL runtime
   # Download from: developer.amd.com/tools-and-sdks/opencl-zone/
   ```

3. **Device Compatibility**
   - Verify AMD GPU supports OpenCL 2.0+
   - Check device ID configuration

### Issue: "Kernel Compilation Errors"

**Symptoms:**
- "OpenCL kernel compilation failed"
- Processing errors
- No frame generation output

**Solutions:**

1. **GPU Driver Issues**
   - Update GPU drivers
   - Reinstall OpenCL runtime
   - Check GPU compatibility

2. **Kernel Issues**
   ```cmd
   # Compile kernels separately
   # Check kernel syntax
   ```

3. **Memory Issues**
   - Increase system memory
   - Check GPU VRAM availability
   - Reduce kernel complexity

## Memory and Resource Problems

### Issue: "Out of GPU Memory"

**Symptoms:**
- "GPU out of memory" errors
- System crashes
- Poor performance

**Solutions:**

1. **Memory Monitoring**
   ```cmd
   # Monitor GPU memory
   nvidia-smi -q -d MEMORY
   
   # Check current usage in overlay
   ```

2. **Memory Pool Adjustment**
   - Reduce memory pool size
   - Enable memory pooling if disabled
   - Clear memory cache regularly

3. **System Memory**
   - Close unnecessary applications
   - Increase virtual memory
   - Restart system if memory fragmented

### Issue: "Resource Contention"

**Symptoms:**
- Performance degrades over time
- Memory fragmentation
- System instability

**Solutions:**

1. **Resource Monitoring**
   - Task Manager → Performance tab
   - GPU monitoring tools
   - CrossLink-FG built-in stats

2. **Regular Maintenance**
   - Restart CrossLink-FG periodically
   - Clear temporary files
   - Monitor resource usage

## Log File Analysis

### Location of Log Files

```
%APPDATA%\CrossLink-FG\logs\
├── runtime.log          # General runtime messages
├── error.log            # Error messages only
├── performance.log      # Performance metrics
├── debug.log            # Debug information
└── installation.log     # Installation process
```

### Common Log Entries

#### Information Messages
```
[INFO] CrossLink-FG initialized successfully
[INFO] GPU detected: NVIDIA RTX 4090
[INFO] GPU detected: AMD RX 7800 XT
[INFO] DirectX 12 context created
[INFO] Frame generation enabled
[INFO] NVIDIA Optical Flow initialized
[INFO] OpenCL platform found
```

#### Warning Messages
```
[WARNING] GPU temperature high: 85°C
[WARNING] Memory usage approaching limit
[WARNING] Frame generation quality reduced
[WARNING] Asynchronous processing disabled
```

#### Error Messages
```
[ERROR] GPU not found: CUDA device 1
[ERROR] DirectX 12 not available
[ERROR] OpenCL initialization failed
[ERROR] Frame processing timeout
[ERROR] Memory allocation failed
[ERROR] Kernel compilation failed
```

### Log Analysis Tools

```cmd
# View recent errors
findstr /R /N "ERROR" %APPDATA%\CrossLink-FG\logs\runtime.log

# Monitor logs in real-time
powershell "Get-Content %APPDATA%\CrossLink-FG\logs\runtime.log -Wait"

# Filter by timestamp
findstr /R /C:"[0-9][0-9]:[0-9][0-9]:[0-9][0-9]" %APPDATA%\CrossLink-FG\logs\runtime.log
```

## Advanced Diagnostics

### System Information Collection

```cmd
# Create system report
CrossLink-FG.exe --diagnostic-report

# This creates: %APPDATA%\CrossLink-FG\system_report.txt
```

### Performance Benchmarking

```cmd
# Run comprehensive benchmark
CrossLink-FG.exe --benchmark --duration 300

# Benchmark specific features
CrossLink-FG.exe --benchmark --feature optical-flow
CrossLink-FG.exe --benchmark --feature opencl
CrossLink-FG.exe --benchmark --feature frame-gen
```

### Memory Leak Detection

```cmd
# Enable memory tracking
CrossLink-FG.exe --debug --memory-tracking

# Check for leaks
CrossLink-FG.exe --check-memory-leaks
```

### Hardware Compatibility Testing

```cmd
# Test GPU compatibility
CrossLink-FG.exe --test-gpu-compatibility

# Test DirectX compatibility
CrossLink-FG.exe --test-directx
```

## Support and Reporting

### Before Contacting Support

1. **Run Diagnostics**
   ```cmd
   CrossLink-FG.exe --full-diagnostic
   ```

2. **Collect Information**
   - Operating system version
   - GPU models and driver versions
   - CrossLink-FG version
   - Log files
   - Screenshot of error

3. **Reproduce Issue**
   - Note exact steps to reproduce
   - Document expected vs actual behavior
   - Test on clean system if possible

### Support Channels

#### GitHub Issues
- **URL:** https://github.com/crosslink-fg/issues
- **For:** Bug reports, feature requests
- **Response:** Typically within 24-48 hours

#### Email Support
- **Address:** support@crosslink-fg.github.io
- **For:** Technical support, licensing questions
- **Include:** Full diagnostic report

#### Documentation
- **User Guide:** [USER_GUIDE.md](USER_GUIDE.md)
- **Developer Guide:** [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)
- **API Reference:** [API_REFERENCE.md](API_REFERENCE.md)

### Issue Reporting Template

When reporting issues, include this information:

```markdown
### System Information
- OS: Windows 10/11 version XXXX
- GPU 1: NVIDIA RTX XXXX, Driver version XXXX
- GPU 2: AMD RX XXXX, Driver version XXXX
- CrossLink-FG Version: 1.0.0

### Issue Description
Clear description of the problem...

### Reproduction Steps
1. Step 1
2. Step 2
3. Step 3

### Expected Behavior
What should happen...

### Actual Behavior
What actually happens...

### Log Files
Attach relevant log files...

### Additional Context
Any other relevant information...
```

### Emergency Procedures

#### Complete Reset
```cmd
# Complete uninstall
CrossLink-FG.exe --uninstall --purge

# Manual cleanup
rmdir /s /q "C:\Program Files\CrossLink-FG"
rmdir /s /q "%APPDATA%\CrossLink-FG"
rmdir /s /q "%LOCALAPPDATA%\CrossLink-FG"
```

#### Safe Mode Operation
```cmd
# Run in minimal mode
CrossLink-FG.exe --safe-mode --no-overlay --no-logging

# Disable all features
CrossLink-FG.exe --disable-all --minimal-mode
```

#### Emergency Recovery
```cmd
# Restore default configuration
CrossLink-FG.exe --restore-defaults

# Reset to factory settings
CrossLink-FG.exe --factory-reset
```

---

This troubleshooting guide covers the most common issues with CrossLink-FG. For issues not covered here, please contact support with detailed diagnostic information.

**Last Resort Solutions:**
1. **Complete reinstall** of CrossLink-FG
2. **Fresh system install** if persistent issues
3. **Hardware upgrade** if system doesn't meet requirements
4. **Professional support** for enterprise deployments
