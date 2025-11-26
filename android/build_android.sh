#!/bin/bash
# Set Android SDK path (update <your-username> as needed)
export ANDROID_SDK_ROOT="/c/Users/hakri/AppData/Local/Android/Sdk"

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
    read -p "Press Enter to continue..."
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
    read -p "Press Enter to continue..."
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
    
    # Find the latest installed build-tools version
    local BUILD_TOOLS_DIR="$ANDROID_SDK_ROOT/build-tools"
    local BUILD_TOOLS_VERSION=""
    if [ -d "$BUILD_TOOLS_DIR" ]; then
        BUILD_TOOLS_VERSION=$(ls -1 "$BUILD_TOOLS_DIR" | sort -V | tail -1)
    fi
    
    if [ -z "$BUILD_TOOLS_VERSION" ]; then
        log_error "No Android build-tools found in $BUILD_TOOLS_DIR"
        return 1
    fi
    
    log_info "Using build-tools version: $BUILD_TOOLS_VERSION"
    log_info "Signing APK..."
    
    # Align the APK
    "$ANDROID_SDK_ROOT/build-tools/$BUILD_TOOLS_VERSION/zipalign" -v -p 4 "$UNSIGNED_APK" "$ALIGNED_APK"
    
    # Sign with apksigner
    "$ANDROID_SDK_ROOT/build-tools/$BUILD_TOOLS_VERSION/apksigner" sign \
        --ks "$KEYSTORE_FILE" \
        --ks-pass "pass:$KEYSTORE_PASSWORD" \
        --ks-key-alias "$KEY_ALIAS" \
        --key-pass "pass:$KEY_PASSWORD" \
        --out "$SIGNED_APK" \
        "$ALIGNED_APK"
    
    rm "$ALIGNED_APK"
    
    log_info "Signed APK: $SIGNED_APK"
}

# verify_assets - Verify that assets are properly copied to the Android assets directory
# This is called after the Python script runs to ensure assets are available.
# If assets are missing, it falls back to manual copy using bash commands.
# Returns: outputs "true" or "false" to stdout
verify_assets() {
    local ANDROID_ASSETS_DIR="$ANDROID_DIR/app/src/main/assets"
    local ASSETS_VALID=true
    
    # Log to stderr so only the result goes to stdout
    log_info "Verifying assets in $ANDROID_ASSETS_DIR..." >&2
    
    # Check if sprites directory exists and has PNG files
    if [ ! -d "$ANDROID_ASSETS_DIR/sprites" ]; then
        log_warn "sprites/ directory missing in Android assets" >&2
        ASSETS_VALID=false
    elif [ -z "$(find "$ANDROID_ASSETS_DIR/sprites" -name "*.png" 2>/dev/null | head -1)" ]; then
        log_warn "No PNG files found in sprites/ directory" >&2
        ASSETS_VALID=false
    else
        local PNG_COUNT=$(find "$ANDROID_ASSETS_DIR/sprites" -name "*.png" | wc -l)
        log_info "Found $PNG_COUNT PNG files in sprites/ directory" >&2
    fi
    
    # Check if asset_list.txt exists
    if [ ! -f "$ANDROID_ASSETS_DIR/asset_list.txt" ]; then
        log_warn "asset_list.txt missing in Android assets" >&2
        ASSETS_VALID=false
    else
        local ASSET_COUNT=$(wc -l < "$ANDROID_ASSETS_DIR/asset_list.txt")
        log_info "asset_list.txt contains $ASSET_COUNT entries" >&2
    fi
    
    echo "$ASSETS_VALID"
}

# fallback_copy_assets - Manually copy assets if Python script failed
# This ensures the APK has assets even if the Python script isn't available.
fallback_copy_assets() {
    local ANDROID_ASSETS_DIR="$ANDROID_DIR/app/src/main/assets"
    
    log_info "Using fallback: manually copying assets to Android assets directory..."
    
    # Create the assets directory if it doesn't exist
    mkdir -p "$ANDROID_ASSETS_DIR"
    
    # Copy sprites directory from project root
    if [ -d "$PROJECT_ROOT/sprites" ]; then
        log_info "Copying sprites/ directory..."
        cp -r "$PROJECT_ROOT/sprites" "$ANDROID_ASSETS_DIR/"
        log_info "sprites/ copied successfully"
    else
        log_error "sprites/ directory not found in project root: $PROJECT_ROOT"
        return 1
    fi
    
    # Copy maps directory if it exists
    if [ -d "$PROJECT_ROOT/maps" ]; then
        log_info "Copying maps/ directory..."
        cp -r "$PROJECT_ROOT/maps" "$ANDROID_ASSETS_DIR/"
        log_info "maps/ copied successfully"
    fi
    
    # Generate asset_list.txt using bash as fallback
    # This lists all PNG files in the sprites/ directory (matching the original format)
    log_info "Generating asset_list.txt..."
    (
        cd "$ANDROID_ASSETS_DIR"
        find sprites -type f -name "*.png" 2>/dev/null | sort
    ) > "$ANDROID_ASSETS_DIR/asset_list.txt"
    
    local ASSET_COUNT=$(wc -l < "$ANDROID_ASSETS_DIR/asset_list.txt")
    log_info "Generated asset_list.txt with $ASSET_COUNT entries"
    
    return 0
}

main() {
    log_info "Starting Android build..."
    log_info "Project root: $PROJECT_ROOT"
    log_info "Android directory: $ANDROID_DIR"
    # Ensure assets and an asset manifest are prepared so the game can load
    # resources on Android (the APK packages assets and they are not visible
    # via normal filesystem APIs). This will copy `sprites/` and `assets/`
    # into the Android assets directory and generate `asset_list.txt`.
    if command -v python3 >/dev/null 2>&1; then
        log_info "Generating asset manifest and copying assets..."
        python3 ../scripts/generate_asset_manifest.py || log_warn "Asset manifest generation failed"
    elif command -v python >/dev/null 2>&1; then
        log_info "Generating asset manifest and copying assets..."
        python ../scripts/generate_asset_manifest.py || log_warn "Asset manifest generation failed"
    else
        log_warn "Python not found; ensure assets/ and sprites/ are copied into android assets manually."
    fi
    
    # Verify that assets were copied successfully
    # This fixes the gray screen issue by ensuring assets are always available
    if [ "$(verify_assets)" != "true" ]; then
        log_warn "Asset verification failed. Attempting fallback copy..."
        if fallback_copy_assets; then
            # Re-verify after fallback
            if [ "$(verify_assets)" != "true" ]; then
                log_error "Assets still missing after fallback copy. APK may show gray screen!"
            else
                log_info "Fallback copy successful. Assets are now available."
            fi
        else
            log_error "Fallback copy failed. APK will likely show gray screen!"
        fi
    else
        log_info "Asset verification passed."
    fi
    
    # Download SDL2 if needed
    download_sdl2

    # Build SDL2 for Android
    build_sdl2_for_android

    # --- Automated native library packaging ---
    log_info "Packaging native libraries into jniLibs..."
    JNI_LIBS_DIR="$ANDROID_DIR/app/src/main/jniLibs"
    for ABI in armeabi-v7a arm64-v8a; do
        mkdir -p "$JNI_LIBS_DIR/$ABI"
        # Copy SDL2
        if [ -f "$SDL2_DOWNLOAD_DIR/SDL2/lib/$ABI/libSDL2.so" ]; then
            cp "$SDL2_DOWNLOAD_DIR/SDL2/lib/$ABI/libSDL2.so" "$JNI_LIBS_DIR/$ABI/"
        fi
        # Copy SDL2_image
        if [ -f "$SDL2_DOWNLOAD_DIR/SDL2_image/lib/$ABI/libSDL2_image.so" ]; then
            cp "$SDL2_DOWNLOAD_DIR/SDL2_image/lib/$ABI/libSDL2_image.so" "$JNI_LIBS_DIR/$ABI/"
        fi
        # Copy your game library (libgame.so)
        if [ -f "$PROJECT_ROOT/obj/$ABI/libgame.so" ]; then
            cp "$PROJECT_ROOT/obj/$ABI/libgame.so" "$JNI_LIBS_DIR/$ABI/"
        else
            log_warn "libgame.so not found for $ABI in obj/$ABI/. Make sure your native build outputs here."
        fi
    done

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
