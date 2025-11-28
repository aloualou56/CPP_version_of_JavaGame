# Android Build and Test Agent

This directory contains automated tooling to build, deploy, and test the Android APK for grass tile color verification.

## Problem Description

Grass tiles render **red** on Android but **green** on Windows. This is caused by pixel format channel order mismatches when creating SDL textures from surfaces on OpenGL ES.

## The Fix

The fix is implemented in `src/TextureManager.cpp`:
- On Android (`__ANDROID__` defined), all surfaces are converted to `SDL_PIXELFORMAT_ABGR8888` before creating textures
- This ensures correct R/G/B channel ordering for OpenGL ES
- Desktop builds remain unchanged

## Prerequisites

### Required Software
- **Android SDK** with platform-tools (adb)
- **Android NDK** (version 23.1.7779620 or compatible)
- **Java JDK** 11 or 17
- **Python 3** with Pillow library (`pip install Pillow`)
- **Connected Android device** or running emulator

### Environment Variables
```bash
export ANDROID_SDK_ROOT="$HOME/Android/Sdk"
export ANDROID_NDK_HOME="$ANDROID_SDK_ROOT/ndk/23.1.7779620"
export JAVA_HOME="/usr/lib/jvm/java-17-openjdk"
export PATH="$PATH:$ANDROID_SDK_ROOT/platform-tools"
```

## Running the Automated Test

### 1. Start an Emulator or Connect a Device
```bash
# Start an emulator (if you have one configured)
$ANDROID_SDK_ROOT/emulator/emulator -avd <your_avd_name> &

# Or connect a physical device via USB with USB debugging enabled
```

### 2. Run the Build and Test Script
```bash
cd android
./build_and_test_agent.sh
```

The script will:
1. Validate environment variables
2. Build the debug APK using Gradle
3. Install the APK on the connected device/emulator
4. Launch the app
5. Capture a screenshot
6. Analyze the screenshot for grass color
7. Report PASS (green) or FAIL (red)

### 3. View Results
- Screenshots are saved in `tests/android_screenshots/`
- Analysis reports are generated alongside each screenshot
- A JSON manifest tracks all test runs

## Manual Testing Steps

If automated testing is unavailable:

1. **Build the APK:**
   ```bash
   cd android
   ./gradlew assembleDebug
   ```

2. **Install on device:**
   ```bash
   adb install -r app/build/outputs/apk/debug/app-debug.apk
   ```

3. **Launch the app:**
   ```bash
   adb shell am start -n com.aloualou.cppgame/org.libsdl.app.SDLActivity
   ```

4. **Take a screenshot:**
   ```bash
   adb exec-out screencap -p > screenshot.png
   ```

5. **Analyze the screenshot:**
   ```bash
   python3 ../tests/check_grass_color.py screenshot.png --verbose
   ```

## Troubleshooting

### No devices found
```bash
adb devices  # Should list at least one device
```
- Ensure USB debugging is enabled on physical devices
- For emulators, wait for full boot before running tests

### Build fails
- Check that JAVA_HOME points to JDK 11 or 17
- Verify NDK is installed: `ls $ANDROID_SDK_ROOT/ndk/`
- Run `./gradlew clean` before rebuilding

### Still seeing red grass
1. Check logcat for texture conversion messages:
   ```bash
   adb logcat | grep -E "(ANDROID|SDL|TextureManager)"
   ```
2. Look for `[ANDROID] Converting texture` messages
3. Ensure the APK was rebuilt after code changes

## CI Integration

For CI environments without emulators, the script will:
1. Fail gracefully with a clear message
2. Document that manual device testing is required
3. Still validate that the APK builds successfully

## Files

- `build_and_test_agent.sh` - Main automation script
- `../tests/check_grass_color.py` - Screenshot analysis tool
- `../tests/android_screenshots/` - Test artifacts storage
