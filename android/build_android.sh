#!/bin/bash
#
# build_android.sh - Build Android APK for SDL2 Game
#
# This script downloads SDL2 Android libraries and builds the APK.
#
# Usage:
#   ./build_android.sh [options]
#
# Options:
#   --clean          Clean build directory before building
#   --debug          Build debug APK instead of release
#   --sign           Sign the APK (requires keystore configuration)
#   --help           Show this help message
#
# Environment Variables:
#   ANDROID_SDK_ROOT     Path to Android SDK (required if not in standard location)
#   ANDROID_NDK_HOME     Path to Android NDK (optional, uses SDK's ndk-bundle if not set)
#   KEYSTORE_FILE        Path to keystore for signing (optional)
#   KEYSTORE_PASSWORD    Keystore password (optional)
#   KEY_ALIAS            Key alias in keystore (optional)
#   KEY_PASSWORD         Key password (optional)
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ANDROID_DIR="$SCRIPT_DIR"
PROJECT_ROOT="$(dirname "$ANDROID_DIR")"

# SDL2 version to download
SDL2_VERSION="2.28.5"
SDL2_IMAGE_VERSION="2.8.2"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
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

show_help() {
    head -30 "$0" | tail -25 | sed 's/^# *//'
    exit 0
}

# Parse arguments
CLEAN=false
BUILD_TYPE="assembleRelease"
SIGN_APK=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN=true
            shift
            ;;
        --debug)
            BUILD_TYPE="assembleDebug"
            shift
            ;;
        --sign)
            SIGN_APK=true
            shift
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

# Check for Android SDK
if [ -z "$ANDROID_SDK_ROOT" ]; then
    # Try common locations
    if [ -d "$HOME/Android/Sdk" ]; then
        export ANDROID_SDK_ROOT="$HOME/Android/Sdk"
    elif [ -d "$HOME/Library/Android/sdk" ]; then
        export ANDROID_SDK_ROOT="$HOME/Library/Android/sdk"
    elif [ -d "/usr/local/android-sdk" ]; then
        export ANDROID_SDK_ROOT="/usr/local/android-sdk"
    else
        log_error "ANDROID_SDK_ROOT not set and Android SDK not found in common locations."
        log_error "Please set ANDROID_SDK_ROOT to your Android SDK installation path."
        exit 1
    fi
fi

log_info "Using Android SDK at: $ANDROID_SDK_ROOT"

# SDL2 download directory
SDL2_DOWNLOAD_DIR="$ANDROID_DIR/app/src/main/cpp"

download_sdl2() {
    log_info "Downloading SDL2 Android libraries..."
    
    local SDL2_URL="https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-${SDL2_VERSION}.tar.gz"
    local SDL2_IMAGE_URL="https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMAGE_VERSION}/SDL2_image-${SDL2_IMAGE_VERSION}.tar.gz"
    
    mkdir -p "$SDL2_DOWNLOAD_DIR/tmp"
    cd "$SDL2_DOWNLOAD_DIR/tmp"
    
    # Download SDL2
    if [ ! -d "$SDL2_DOWNLOAD_DIR/SDL2" ]; then
        log_info "Downloading SDL2 ${SDL2_VERSION}..."
        curl -L -o SDL2.tar.gz "$SDL2_URL"
        tar -xzf SDL2.tar.gz
        
        # Copy Android prebuilt libraries
        mkdir -p "$SDL2_DOWNLOAD_DIR/SDL2/include"
        mkdir -p "$SDL2_DOWNLOAD_DIR/SDL2/lib"
        
        cp -r "SDL2-${SDL2_VERSION}/include/"* "$SDL2_DOWNLOAD_DIR/SDL2/include/"
        
        # Copy Java SDL source files
        mkdir -p "$ANDROID_DIR/app/src/main/java/org/libsdl/app"
        cp "SDL2-${SDL2_VERSION}/android-project/app/src/main/java/org/libsdl/app/"*.java \
           "$ANDROID_DIR/app/src/main/java/org/libsdl/app/"
        
        log_info "SDL2 downloaded and extracted."
    else
        log_info "SDL2 already exists, skipping download."
    fi
    
    # Download SDL2_image
    if [ ! -d "$SDL2_DOWNLOAD_DIR/SDL2_image" ]; then
        log_info "Downloading SDL2_image ${SDL2_IMAGE_VERSION}..."
        curl -L -o SDL2_image.tar.gz "$SDL2_IMAGE_URL"
        tar -xzf SDL2_image.tar.gz
        
        mkdir -p "$SDL2_DOWNLOAD_DIR/SDL2_image/include"
        mkdir -p "$SDL2_DOWNLOAD_DIR/SDL2_image/lib"
        
        cp -r "SDL2_image-${SDL2_IMAGE_VERSION}/include/"* "$SDL2_DOWNLOAD_DIR/SDL2_image/include/"
        
        log_info "SDL2_image downloaded and extracted."
    else
        log_info "SDL2_image already exists, skipping download."
    fi
    
    cd "$ANDROID_DIR"
    rm -rf "$SDL2_DOWNLOAD_DIR/tmp"
}

build_sdl2_for_android() {
    log_info "Building SDL2 for Android..."
    
    # Check if we need to build
    if [ -f "$SDL2_DOWNLOAD_DIR/SDL2/lib/arm64-v8a/libSDL2.so" ] && \
       [ -f "$SDL2_DOWNLOAD_DIR/SDL2/lib/armeabi-v7a/libSDL2.so" ]; then
        log_info "SDL2 libraries already built, skipping."
        return
    fi
    
    # We need the NDK to build
    if [ -z "$ANDROID_NDK_HOME" ]; then
        if [ -d "$ANDROID_SDK_ROOT/ndk/23.1.7779620" ]; then
            export ANDROID_NDK_HOME="$ANDROID_SDK_ROOT/ndk/23.1.7779620"
        elif [ -d "$ANDROID_SDK_ROOT/ndk-bundle" ]; then
            export ANDROID_NDK_HOME="$ANDROID_SDK_ROOT/ndk-bundle"
        else
            log_warn "NDK not found. Installing via sdkmanager..."
            "$ANDROID_SDK_ROOT/cmdline-tools/latest/bin/sdkmanager" --install "ndk;23.1.7779620"
            export ANDROID_NDK_HOME="$ANDROID_SDK_ROOT/ndk/23.1.7779620"
        fi
    fi
    
    log_info "Using NDK at: $ANDROID_NDK_HOME"
    
    # Download prebuilt SDL2 for Android from a release
    local PREBUILT_URL="https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-${SDL2_VERSION}-android.zip"
    local PREBUILT_IMAGE_URL="https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMAGE_VERSION}/SDL2_image-${SDL2_IMAGE_VERSION}-android.zip"
    
    mkdir -p "$SDL2_DOWNLOAD_DIR/tmp"
    cd "$SDL2_DOWNLOAD_DIR/tmp"
    
    log_info "Downloading prebuilt SDL2 for Android..."
    curl -L -o SDL2-android.zip "$PREBUILT_URL" || {
        log_warn "Prebuilt SDL2 not available, will build from source during Gradle build."
        cd "$ANDROID_DIR"
        return
    }
    
    unzip -o SDL2-android.zip
    
    # Copy prebuilt libraries
    for ABI in armeabi-v7a arm64-v8a; do
        mkdir -p "$SDL2_DOWNLOAD_DIR/SDL2/lib/$ABI"
        if [ -f "SDL2-${SDL2_VERSION}/lib/$ABI/libSDL2.so" ]; then
            cp "SDL2-${SDL2_VERSION}/lib/$ABI/libSDL2.so" "$SDL2_DOWNLOAD_DIR/SDL2/lib/$ABI/"
        fi
    done
    
    log_info "Downloading prebuilt SDL2_image for Android..."
    curl -L -o SDL2_image-android.zip "$PREBUILT_IMAGE_URL" || {
        log_warn "Prebuilt SDL2_image not available."
    }
    
    if [ -f SDL2_image-android.zip ]; then
        unzip -o SDL2_image-android.zip
        for ABI in armeabi-v7a arm64-v8a; do
            mkdir -p "$SDL2_DOWNLOAD_DIR/SDL2_image/lib/$ABI"
            if [ -f "SDL2_image-${SDL2_IMAGE_VERSION}/lib/$ABI/libSDL2_image.so" ]; then
                cp "SDL2_image-${SDL2_IMAGE_VERSION}/lib/$ABI/libSDL2_image.so" "$SDL2_DOWNLOAD_DIR/SDL2_image/lib/$ABI/"
            fi
        done
    fi
    
    cd "$ANDROID_DIR"
    rm -rf "$SDL2_DOWNLOAD_DIR/tmp"
    
    log_info "SDL2 libraries ready."
}

clean_build() {
    log_info "Cleaning build directory..."
    cd "$ANDROID_DIR"
    ./gradlew clean
}

build_apk() {
    log_info "Building APK..."
    cd "$ANDROID_DIR"
    
    if [ "$BUILD_TYPE" == "assembleDebug" ]; then
        ./gradlew assembleDebug
        log_info "Debug APK built: app/build/outputs/apk/debug/app-debug.apk"
    else
        ./gradlew assembleRelease
        log_info "Release APK built: app/build/outputs/apk/release/app-release-unsigned.apk"
    fi
}

sign_apk() {
    if [ "$BUILD_TYPE" == "assembleDebug" ]; then
        log_info "Debug builds are automatically signed."
        return
    fi
    
    if [ -z "$KEYSTORE_FILE" ]; then
        log_warn "KEYSTORE_FILE not set. APK will remain unsigned."
        log_info "To sign the APK, set these environment variables:"
        log_info "  KEYSTORE_FILE      - Path to your .keystore file"
        log_info "  KEYSTORE_PASSWORD  - Password for the keystore"
        log_info "  KEY_ALIAS          - Alias of the signing key"
        log_info "  KEY_PASSWORD       - Password for the key"
        return
    fi
    
    local UNSIGNED_APK="$ANDROID_DIR/app/build/outputs/apk/release/app-release-unsigned.apk"
    local SIGNED_APK="$ANDROID_DIR/app/build/outputs/apk/release/app-release.apk"
    local ALIGNED_APK="$ANDROID_DIR/app/build/outputs/apk/release/app-release-aligned.apk"
    
    log_info "Signing APK..."
    
    # Align the APK
    "$ANDROID_SDK_ROOT/build-tools/34.0.0/zipalign" -v -p 4 "$UNSIGNED_APK" "$ALIGNED_APK"
    
    # Sign with apksigner
    "$ANDROID_SDK_ROOT/build-tools/34.0.0/apksigner" sign \
        --ks "$KEYSTORE_FILE" \
        --ks-pass "pass:$KEYSTORE_PASSWORD" \
        --ks-key-alias "$KEY_ALIAS" \
        --key-pass "pass:$KEY_PASSWORD" \
        --out "$SIGNED_APK" \
        "$ALIGNED_APK"
    
    rm "$ALIGNED_APK"
    
    log_info "Signed APK: $SIGNED_APK"
}

main() {
    log_info "Starting Android build..."
    log_info "Project root: $PROJECT_ROOT"
    log_info "Android directory: $ANDROID_DIR"
    
    # Download SDL2 if needed
    download_sdl2
    
    # Build SDL2 for Android
    build_sdl2_for_android
    
    # Clean if requested
    if [ "$CLEAN" = true ]; then
        clean_build
    fi
    
    # Build the APK
    build_apk
    
    # Sign if requested
    if [ "$SIGN_APK" = true ]; then
        sign_apk
    fi
    
    log_info "Build complete!"
}

main
