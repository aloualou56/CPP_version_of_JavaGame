#!/bin/bash
#
# build_and_test_agent.sh - Automated Android build and test script
#
# This script builds the Android APK, installs it on a device/emulator,
# captures screenshots, and analyzes them to detect grass tile color issues.
#
# Usage:
#   ./build_and_test_agent.sh [options]
#
# Options:
#   --skip-build       Skip APK build (use existing APK)
#   --emulator NAME    Use specific emulator (default: first connected device)
#   --iterations N     Max iterations for fix attempts (default: 2)
#   --help             Show this help message
#
# Prerequisites:
#   - ANDROID_SDK_ROOT  Path to Android SDK
#   - ANDROID_NDK_HOME  Path to Android NDK (optional, auto-detected)
#   - JAVA_HOME         Path to JDK 11 or 17
#   - adb               Must be in PATH or in ANDROID_SDK_ROOT/platform-tools
#   - Python 3          For screenshot analysis (with Pillow library)
#
# Environment Setup Example (Linux/macOS):
#   export ANDROID_SDK_ROOT="$HOME/Android/Sdk"
#   export ANDROID_NDK_HOME="$ANDROID_SDK_ROOT/ndk/25.2.9519653"
#   export JAVA_HOME="/usr/lib/jvm/java-17-openjdk"
#   export PATH="$PATH:$ANDROID_SDK_ROOT/platform-tools"
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ANDROID_DIR="$SCRIPT_DIR"
PROJECT_ROOT="$(dirname "$ANDROID_DIR")"
TESTS_DIR="$PROJECT_ROOT/tests"
SCREENSHOTS_DIR="$TESTS_DIR/android_screenshots"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_step() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

show_help() {
    head -40 "$0" | tail -35 | sed 's/^# *//'
    exit 0
}

# Default options
SKIP_BUILD=false
EMULATOR_NAME=""
MAX_ITERATIONS=2

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --skip-build)
            SKIP_BUILD=true
            shift
            ;;
        --emulator)
            EMULATOR_NAME="$2"
            shift 2
            ;;
        --iterations)
            MAX_ITERATIONS="$2"
            shift 2
            ;;
        --help)
            show_help
            ;;
        *)
            log_error "Unknown option: $1"
            show_help
            ;;
    esac
done

# Check required environment variables
check_environment() {
    log_step "Checking environment..."
    
    local missing_vars=()
    
    # Check ANDROID_SDK_ROOT
    if [ -z "$ANDROID_SDK_ROOT" ]; then
        # Try to auto-detect common locations
        if [ -d "$HOME/Android/Sdk" ]; then
            export ANDROID_SDK_ROOT="$HOME/Android/Sdk"
            log_info "Auto-detected ANDROID_SDK_ROOT: $ANDROID_SDK_ROOT"
        elif [ -d "$HOME/Library/Android/sdk" ]; then
            export ANDROID_SDK_ROOT="$HOME/Library/Android/sdk"
            log_info "Auto-detected ANDROID_SDK_ROOT: $ANDROID_SDK_ROOT"
        else
            missing_vars+=("ANDROID_SDK_ROOT")
        fi
    fi
    
    # Check JAVA_HOME
    if [ -z "$JAVA_HOME" ]; then
        # Try to detect Java
        if command -v java &>/dev/null; then
            local java_path=$(which java)
            # Try to resolve JAVA_HOME from java command
            if [ -f "/usr/libexec/java_home" ]; then
                export JAVA_HOME=$(/usr/libexec/java_home 2>/dev/null || true)
            elif [ -L "$java_path" ]; then
                export JAVA_HOME=$(dirname $(dirname $(readlink -f "$java_path")))
            fi
            if [ -n "$JAVA_HOME" ]; then
                log_info "Auto-detected JAVA_HOME: $JAVA_HOME"
            else
                missing_vars+=("JAVA_HOME")
            fi
        else
            missing_vars+=("JAVA_HOME")
        fi
    fi
    
    # Check ANDROID_NDK_HOME (optional but warn)
    if [ -z "$ANDROID_NDK_HOME" ]; then
        if [ -d "$ANDROID_SDK_ROOT/ndk" ]; then
            # Get the latest NDK version
            export ANDROID_NDK_HOME=$(ls -d "$ANDROID_SDK_ROOT/ndk"/* 2>/dev/null | sort -V | tail -1)
            if [ -n "$ANDROID_NDK_HOME" ]; then
                log_info "Auto-detected ANDROID_NDK_HOME: $ANDROID_NDK_HOME"
            fi
        fi
        if [ -z "$ANDROID_NDK_HOME" ]; then
            log_warn "ANDROID_NDK_HOME not set. Native build may fail if NDK is required."
        fi
    fi
    
    # Fail if required variables are missing
    if [ ${#missing_vars[@]} -gt 0 ]; then
        log_error "Missing required environment variables:"
        for var in "${missing_vars[@]}"; do
            log_error "  - $var"
        done
        log_error ""
        log_error "Please set these variables before running this script."
        log_error "Example:"
        log_error "  export ANDROID_SDK_ROOT=\"\$HOME/Android/Sdk\""
        log_error "  export JAVA_HOME=\"/usr/lib/jvm/java-17-openjdk\""
        exit 1
    fi
    
    # Add platform-tools to PATH if not already
    if ! command -v adb &>/dev/null; then
        if [ -f "$ANDROID_SDK_ROOT/platform-tools/adb" ]; then
            export PATH="$PATH:$ANDROID_SDK_ROOT/platform-tools"
            log_info "Added platform-tools to PATH"
        else
            log_error "adb not found. Please install Android platform-tools."
            exit 1
        fi
    fi
    
    # Check Python
    if ! command -v python3 &>/dev/null && ! command -v python &>/dev/null; then
        log_error "Python 3 is required for screenshot analysis."
        log_error "Please install Python 3 and the Pillow library."
        exit 1
    fi
    
    log_info "Environment check passed."
    log_info "  ANDROID_SDK_ROOT: $ANDROID_SDK_ROOT"
    log_info "  ANDROID_NDK_HOME: ${ANDROID_NDK_HOME:-'(not set)'}"
    log_info "  JAVA_HOME: $JAVA_HOME"
}

# Build the APK
build_apk() {
    if [ "$SKIP_BUILD" = true ]; then
        log_info "Skipping build (--skip-build specified)"
        return 0
    fi
    
    log_step "Building Android APK..."
    cd "$ANDROID_DIR"
    
    # Run gradlew with debug build
    ./gradlew assembleDebug --stacktrace || {
        log_error "Gradle build failed. Check logs above for details."
        exit 1
    }
    
    # Verify APK was created
    local apk_path="$ANDROID_DIR/app/build/outputs/apk/debug/app-debug.apk"
    if [ ! -f "$apk_path" ]; then
        log_error "APK not found at: $apk_path"
        exit 1
    fi
    
    log_info "APK built successfully: $apk_path"
}

# Get device/emulator serial
get_device() {
    local devices=$(adb devices | grep -v "List of devices" | grep -v "^$" | awk '{print $1}')
    local device_count=$(echo "$devices" | grep -c . || true)
    
    if [ -z "$devices" ] || [ "$device_count" -eq 0 ]; then
        log_error "No connected devices or emulators found."
        log_error "Please connect a device or start an emulator."
        log_error ""
        log_error "To start an emulator:"
        log_error "  \$ANDROID_SDK_ROOT/emulator/emulator -avd <avd_name>"
        exit 1
    fi
    
    if [ -n "$EMULATOR_NAME" ]; then
        # Try to find specific emulator
        local found=$(echo "$devices" | grep "$EMULATOR_NAME" || true)
        if [ -z "$found" ]; then
            log_warn "Specified emulator '$EMULATOR_NAME' not found. Using first available device."
            DEVICE_SERIAL=$(echo "$devices" | head -1)
        else
            DEVICE_SERIAL="$found"
        fi
    else
        DEVICE_SERIAL=$(echo "$devices" | head -1)
    fi
    
    log_info "Using device: $DEVICE_SERIAL"
}

# Install APK to device
install_apk() {
    log_step "Installing APK to device..."
    
    local apk_path="$ANDROID_DIR/app/build/outputs/apk/debug/app-debug.apk"
    if [ ! -f "$apk_path" ]; then
        log_error "APK not found: $apk_path"
        exit 1
    fi
    
    adb -s "$DEVICE_SERIAL" install -r "$apk_path" || {
        log_error "Failed to install APK"
        exit 1
    }
    
    log_info "APK installed successfully"
}

# Launch the app and wait for it to start
launch_app() {
    log_step "Launching app..."
    
    # Package name from AndroidManifest.xml
    local package="com.example.sdlgame"
    local activity="SDLActivity"
    
    # Clear any previous logcat
    adb -s "$DEVICE_SERIAL" logcat -c
    
    # Launch the app
    adb -s "$DEVICE_SERIAL" shell am start -n "$package/org.libsdl.app.$activity" || {
        log_error "Failed to launch app"
        exit 1
    }
    
    # Wait for app to fully launch and render
    log_info "Waiting for app to initialize (5 seconds)..."
    sleep 5
    
    log_info "App launched"
}

# Capture screenshot
capture_screenshot() {
    log_step "Capturing screenshot..."
    
    # Create screenshots directory
    mkdir -p "$SCREENSHOTS_DIR"
    
    # Generate timestamp
    local timestamp=$(date +%Y%m%d_%H%M%S)
    local commit_sha=$(git -C "$PROJECT_ROOT" rev-parse --short HEAD 2>/dev/null || echo "unknown")
    local screenshot_name="screenshot_${timestamp}_${commit_sha}.png"
    local screenshot_path="$SCREENSHOTS_DIR/$screenshot_name"
    
    # Capture screenshot using adb
    adb -s "$DEVICE_SERIAL" exec-out screencap -p > "$screenshot_path" || {
        log_error "Failed to capture screenshot"
        exit 1
    }
    
    # Verify screenshot was captured
    if [ ! -s "$screenshot_path" ]; then
        log_error "Screenshot file is empty or was not created"
        exit 1
    fi
    
    # Create/update manifest
    local manifest_path="$SCREENSHOTS_DIR/manifest.json"
    local device_model=$(adb -s "$DEVICE_SERIAL" shell getprop ro.product.model | tr -d '\r')
    local android_version=$(adb -s "$DEVICE_SERIAL" shell getprop ro.build.version.release | tr -d '\r')
    
    # Append to manifest (create if not exists)
    if [ ! -f "$manifest_path" ]; then
        echo "[]" > "$manifest_path"
    fi
    
    # Use Python to update JSON manifest
    python3 -c "
import json
import sys

manifest_path = '$manifest_path'
entry = {
    'filename': '$screenshot_name',
    'timestamp': '$timestamp',
    'commit_sha': '$commit_sha',
    'device_serial': '$DEVICE_SERIAL',
    'device_model': '$device_model',
    'android_version': '$android_version'
}

try:
    with open(manifest_path, 'r') as f:
        manifest = json.load(f)
except:
    manifest = []

manifest.append(entry)

with open(manifest_path, 'w') as f:
    json.dump(manifest, f, indent=2)
" 2>/dev/null || log_warn "Failed to update manifest JSON"
    
    log_info "Screenshot saved: $screenshot_path"
    echo "$screenshot_path"
}

# Analyze screenshot for grass color
analyze_screenshot() {
    local screenshot_path="$1"
    
    log_step "Analyzing screenshot for grass color..."
    
    local check_script="$TESTS_DIR/check_grass_color.py"
    
    if [ ! -f "$check_script" ]; then
        log_error "Analysis script not found: $check_script"
        exit 1
    fi
    
    # Run the analysis script
    local python_cmd="python3"
    if ! command -v python3 &>/dev/null; then
        python_cmd="python"
    fi
    
    $python_cmd "$check_script" "$screenshot_path"
    local result=$?
    
    if [ $result -eq 0 ]; then
        log_info "✓ Grass tiles are GREEN (PASS)"
        return 0
    else
        log_warn "✗ Grass tiles are RED (FAIL)"
        return 1
    fi
}

# Capture logcat for debugging
capture_logs() {
    log_step "Capturing device logs..."
    
    local timestamp=$(date +%Y%m%d_%H%M%S)
    local log_path="$SCREENSHOTS_DIR/logcat_${timestamp}.txt"
    
    # Capture recent logcat, filtered by SDL and app tags
    adb -s "$DEVICE_SERIAL" logcat -d -v time | grep -E "(SDL|SDLActivity|TextureManager|ANDROID)" > "$log_path" 2>/dev/null || true
    
    # Also capture full logcat
    local full_log_path="$SCREENSHOTS_DIR/logcat_full_${timestamp}.txt"
    adb -s "$DEVICE_SERIAL" logcat -d > "$full_log_path" 2>/dev/null || true
    
    log_info "Logs saved to: $log_path"
    log_info "Full logs saved to: $full_log_path"
}

# Main execution
main() {
    log_info "========================================"
    log_info "Android Build and Test Agent"
    log_info "========================================"
    log_info ""
    
    # Step 1: Check environment
    check_environment
    
    # Step 2: Build APK
    build_apk
    
    # Step 3: Get device
    get_device
    
    # Step 4: Install APK
    install_apk
    
    # Step 5: Launch app
    launch_app
    
    # Step 6: Capture screenshot
    local screenshot_path=$(capture_screenshot)
    
    # Step 7: Analyze screenshot
    if analyze_screenshot "$screenshot_path"; then
        log_info ""
        log_info "========================================"
        log_info "SUCCESS: Grass tiles are rendering correctly (GREEN)"
        log_info "========================================"
        exit 0
    else
        log_warn ""
        log_warn "========================================"
        log_warn "FAIL: Grass tiles are rendering incorrectly (RED)"
        log_warn "========================================"
        
        # Capture logs for debugging
        capture_logs
        
        exit 1
    fi
}

main "$@"
