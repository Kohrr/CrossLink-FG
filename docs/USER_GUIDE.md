# CrossLink-FG User Guide

**Version:** 1.0.0  
**Last Updated:** December 4, 2025  
**Target Audience:** End Users, Game Enthusiasts, Performance Tweakers

## Table of Contents

1. [Introduction](#introduction)
2. [Quick Start Guide](#quick-start-guide)
3. [Interface Overview](#interface-overview)
4. [Configuration Settings](#configuration-settings)
5. [Performance Optimization](#performance-optimization)
6. [Gaming Features](#gaming-features)
7. [Troubleshooting](#troubleshooting)
8. [Frequently Asked Questions](#frequently-asked-questions)

## Introduction

CrossLink-FG (Frame Generation Middleware) is a professional-grade solution that enhances gaming performance through advanced cross-GPU frame generation. By utilizing NVIDIA RTX Optical Flow and AMD RDNA compute capabilities, CrossLink-FG generates intermediate frames to achieve smooth, high-FPS gameplay in supported DirectX 12 games.

### Key Features

- **Cross-GPU Frame Generation**: Leverages both NVIDIA and AMD GPUs
- **Real-time ImGui Overlay**: Monitor performance and adjust settings
- **Automatic Game Detection**: Seamless integration with supported games
- **Performance Optimization**: Advanced algorithms for minimal latency
- **Professional UI**: Clean, customizable interface
- **Real-time Monitoring**: FPS, GPU usage, and system metrics

### Supported Games

- **Borderlands 3** (DirectX 12 mode)
- **Borderlands 4** (DirectX 12 mode)

## Quick Start Guide

### First Time Setup

1. **Launch CrossLink-FG**
   ```bash
   # Run as administrator for first time
   cd "C:\Program Files\CrossLink-FG\"
   CrossLink-FG.exe
   ```

2. **Initial Configuration**
   - The overlay will appear in the top-left corner
   - Press `F12` to toggle the configuration panel
   - Use the mouse to navigate and adjust settings

3. **Start Gaming**
   - Launch your supported game
   - CrossLink-FG will automatically detect and attach
   - Monitor performance improvements in real-time

### Basic Controls

| Key | Action |
|-----|--------|
| `F12` | Toggle configuration panel |
| `F11` | Toggle overlay visibility |
| `F10` | Reset to defaults |
| `F9` | Toggle performance logging |
| `Esc` | Close configuration panel |

## Interface Overview

### Main Overlay

The main overlay appears in the top-left corner and displays:

```
┌─────────────────────────────────┐
│ CrossLink-FG v1.0.0            │
│ ┌─────────┬──────┬──────┬─────┐ │
│ │   FPS   │ GPU  │ Mem │ CPU │ │
│ │   144   │ 85% │ 67% │ 34% │ │
│ └─────────┴──────┴──────┴─────┘ │
│ Frame Gen: Active │ Q: High    │
└─────────────────────────────────┘
```

### Configuration Panel

Press `F12` to access the full configuration panel:

```
┌─────────────────────────────────────────────────────┐
│ CrossLink-FG Configuration                          │
├─────────────────────────────────────────────────────┤
│ [General]                                            │
│ ☑ Enable Frame Generation                          │
│ Quality: [High        ▼] Target FPS: [60     ]      │
│                                                         │
│ [Graphics]                                            │
│ Primary GPU: [NVIDIA RTX 4090      ▼]                │
│ Secondary GPU: [AMD RX 7800 XT    ▼]                 │
│ ☐ Use Nvidia Optical Flow                           │
│                                                         │
│ [Performance]                                         │
│ ☑ Async Processing                                   │
│ Thread Priority: [High       ▼]                       │
│ Memory Pool: [2048 MB        ]                       │
│                                                         │
│ [Save] [Reset] [Advanced] [Close]                   │
└─────────────────────────────────────────────────────┘
```

## Configuration Settings

### General Settings

#### Enable Frame Generation
- **Enabled**: Activates frame generation
- **Disabled**: Uses native game rendering only
- **Default**: Enabled

#### Frame Generation Quality
- **Ultra**: Maximum quality, higher GPU usage
- **High**: Balanced quality/performance (recommended)
- **Medium**: Lower quality, better performance
- **Low**: Minimal quality impact, maximum performance
- **Custom**: User-defined settings

#### Target Frame Rate
- **Range**: 30-240 FPS
- **Recommendation**: 60 FPS for most users
- **High-end**: 144 FPS for competitive gaming
- **Default**: 60 FPS

### Graphics Settings

#### Primary GPU
- Automatically detects NVIDIA RTX GPUs
- **Recommended**: RTX 20xx series or newer
- **Fallback**: Intel Arc GPUs (limited support)

#### Secondary GPU
- Automatically detects AMD RDNA GPUs
- **Recommended**: RX 6000/7000 series
- **Alternative**: Second NVIDIA GPU

#### Use Nvidia Optical Flow
- **Enabled**: Uses hardware optical flow
- **Disabled**: Software-based flow estimation
- **Performance Impact**: Hardware is significantly faster
- **Compatibility**: RTX 20xx+ only

### Performance Settings

#### Async Processing
- **Enabled**: Non-blocking frame generation
- **Disabled**: Synchronous processing
- **Recommendation**: Always enabled
- **Impact**: Reduces input latency

#### Thread Priority
- **Realtime**: Maximum CPU priority
- **High**: Above-normal priority (recommended)
- **Normal**: Standard priority
- **Below**: Below-normal priority

#### Memory Pool Size
- **Range**: 512-8192 MB
- **Recommendation**: 25% of total GPU VRAM
- **Minimum**: 1GB for 1080p
- **Recommended**: 2GB+ for 4K

## Performance Optimization

### Recommended Settings by Resolution

#### 1080p (1920x1080)
```
Quality: High
Target FPS: 60
Async Processing: Enabled
Thread Priority: High
Memory Pool: 1024 MB
```

#### 1440p (2560x1440)
```
Quality: High
Target FPS: 60
Async Processing: Enabled
Thread Priority: High
Memory Pool: 2048 MB
```

#### 4K (3840x2160)
```
Quality: Medium
Target FPS: 60
Async Processing: Enabled
Thread Priority: High
Memory Pool: 4096 MB
```

### GPU-Specific Optimizations

#### RTX 4090 + RX 7800 XT
- Enable both GPUs
- Use Nvidia Optical Flow
- Target 144 FPS

#### RTX 3080 + RX 6700 XT
- Enable both GPUs
- Use Nvidia Optical Flow
- Target 120 FPS

#### RTX 3070 + RX 6600 XT
- Enable both GPUs
- Use Nvidia Optical Flow
- Target 90 FPS

### Performance Monitoring

#### Key Metrics
- **Frame Generation Ratio**: Generated frames vs native frames
- **GPU Utilization**: Load on both GPUs
- **Memory Usage**: VRAM consumption
- **Input Latency**: End-to-end delay

#### Performance Tools
- Built-in overlay (F11 to toggle)
- GPU-Z monitoring
- NVIDIA-SMI
- MSI Afterburner

## Gaming Features

### Borderlands 3 Optimization

#### Graphics Settings
```
Graphics API: DirectX 12
V-Sync: Disabled
Motion Blur: Disabled
Depth of Field: Low
Texture Streaming Budget: High
```

#### In-game Settings
```
Resolution: Native (1920x1080, 2560x1440, 3840x2160)
Window Mode: Fullscreen
FPS Cap: Disabled
High Refresh Rate: Enabled
```

### Borderlands 4 Optimization

#### Recommended Settings
```
DX12 Mode: Enabled
Ray Tracing: Disabled (for better frame generation)
DLSS: Disabled (use frame generation instead)
Sharpening: Low
```

#### Performance Tips
- Use frame generation instead of DLSS
- Disable ray tracing for better compatibility
- Enable High Refresh Rate monitor support

### Advanced Gaming Features

#### Automatic Game Detection
- CrossLink-FG automatically detects supported games
- No manual injection required
- Seamless mode switching

#### Session Recording
- Performance logging during gameplay
- Export data to CSV for analysis
- Average FPS and frame time metrics

#### Custom Overlays
- Hide/show specific metrics
- Color customization
- Position adjustment
- Size scaling

## Troubleshooting

### Common Issues

#### Issue: Low FPS with Frame Generation
**Symptoms:**
- FPS lower than native rendering
- High GPU utilization on both cards
- Stuttering or frame drops

**Solutions:**
1. Reduce frame generation quality
2. Lower target FPS
3. Increase memory pool size
4. Update graphics drivers

#### Issue: Overlay Not Visible
**Symptoms:**
- No overlay appears in games
- CrossLink-FG is running but not visible

**Solutions:**
1. Check `OverlayEnabled=1` in config
2. Verify game is using DirectX 12
3. Run both game and CrossLink-FG as administrator
4. Check Windows compatibility mode

#### Issue: Game Crashes
**Symptoms:**
- Game crashes on startup
- Error messages related to DirectX
- System becomes unresponsive

**Solutions:**
1. Remove CrossLink-FG DLL from game directory
2. Update graphics drivers
3. Verify DirectX 12 installation
4. Check Windows for updates

#### Issue: Poor Performance
**Symptoms:**
- Performance worse than expected
- High CPU usage
- One GPU underutilized

**Solutions:**
1. Enable async processing
2. Adjust thread priority
3. Check GPU compatibility
4. Verify power management settings

### Diagnostic Tools

#### System Check
```bash
CrossLink-FG.exe --check-system
```

#### Performance Test
```bash
CrossLink-FG.exe --test-performance
```

#### Log Analysis
```bash
# View logs
notepad %APPDATA%\CrossLink-FG\logs\runtime.log

# Check errors
notepad %APPDATA%\CrossLink-FG\logs\error.log
```

### Error Codes

| Code | Description | Solution |
|------|-------------|----------|
| 1001 | GPU not supported | Update graphics drivers |
| 1002 | DirectX 12 not found | Install latest DirectX Runtime |
| 1003 | Insufficient VRAM | Reduce memory pool size |
| 1004 | Thread creation failed | Run as administrator |
| 1005 | OpenCL initialization failed | Update GPU drivers |

## Frequently Asked Questions

### General Questions

**Q: What games are supported?**
A: Currently Borderlands 3 and Borderlands 4 in DirectX 12 mode. Additional games are planned for future releases.

**Q: Do I need both NVIDIA and AMD GPUs?**
A: Yes, CrossLink-FG is designed for cross-GPU operation. Both a NVIDIA RTX and AMD RDNA GPU are required.

**Q: Will frame generation increase input latency?**
A: When properly configured with async processing, latency increase is minimal (typically 1-3ms).

**Q: Can I use CrossLink-FG with other overlay software?**
A: Some overlays may conflict. Test in your setup and disable problematic overlays if issues occur.

### Performance Questions

**Q: What FPS improvement can I expect?**
A: Typical improvements range from 50-100% depending on game, hardware, and settings.

**Q: How much VRAM does CrossLink-FG use?**
A: Typically 1-4GB depending on resolution and settings.

**Q: Does frame generation work with VR?**
A: Currently not supported in VR mode. Only traditional desktop gaming is supported.

**Q: Can I use CrossLink-FG with DLSS?**
A: Not recommended. Use frame generation instead of DLSS for optimal results.

### Technical Questions

**Q: How does cross-GPU frame generation work?**
A: NVIDIA GPU captures and analyzes frames, AMD GPU generates intermediate frames using OpenCL.

**Q: What is the minimum system requirements?**
A: See the [Installation Guide](INSTALLATION.md) for detailed requirements.

**Q: Can I modify the source code?**
A: Yes, source code is included for research and development purposes.

**Q: Is CrossLink-FG compatible with anti-cheat systems?**
A: Currently in testing. Use at your own risk with competitive games.

### Support Questions

**Q: Where can I get help?**
A: See the [Troubleshooting Guide](TROUBLESHOOTING.md) or contact support at support@crosslink-fg.github.io

**Q: How do I report bugs?**
A: Use the GitHub Issues page at https://github.com/crosslink-fg/issues

**Q: Can I contribute to the project?**
A: Yes! See the [Developer Guide](DEVELOPER_GUIDE.md) for contribution guidelines.

**Q: Will there be future updates?**
A: Yes, regular updates are planned for new games and performance improvements.

### Legal and License Questions

**Q: Is CrossLink-FG legal to use?**
A: Yes, for personal use. Commercial use may require additional licensing.

**Q: Can game developers block CrossLink-FG?**
A: Yes, game developers can detect and block third-party modifications.

**Q: Does using CrossLink-FG violate EULAs?**
A: Review your game's EULA. Use at your own risk.

**Q: Who owns the generated frames?**
A: The technology is yours to use, but respect game publisher intellectual property.

### Best Practices

1. **Always backup game files** before installing
2. **Test in single-player mode** first
3. **Monitor system temperatures** during extended use
4. **Keep drivers updated** for optimal performance
5. **Use recommended settings** as starting point
6. **Test with different quality settings** to find optimal balance
7. **Monitor memory usage** and adjust pool size as needed
8. **Use full-screen mode** for best performance
9. **Disable conflicting overlays** and utilities
10. **Regularly check for updates** and new features

---

For more technical information, see the [Developer Guide](DEVELOPER_GUIDE.md) and [API Reference](API_REFERENCE.md).
