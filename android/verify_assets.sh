#!/bin/bash
#
# verify_assets.sh - Verify that game assets are properly packaged for Android
#
# This script checks if assets are correctly placed in the Android assets
# directory before building the APK. It helps diagnose the gray screen issue
# which is often caused by missing assets.
#
# Usage:
#   ./verify_assets.sh
#
# Exit codes:
#   0 - All assets verified successfully
#   1 - Some assets are missing
#

# Don't use set -e because we want to count errors and continue
# set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ANDROID_DIR="$SCRIPT_DIR"
PROJECT_ROOT="$(dirname "$ANDROID_DIR")"
ASSETS_DIR="$ANDROID_DIR/app/src/main/assets"

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

ERRORS=0

echo "============================================"
echo "  Android Asset Verification Script"
echo "============================================"
echo ""
echo "Checking: $ASSETS_DIR"
echo ""

# Check if assets directory exists
if [ ! -d "$ASSETS_DIR" ]; then
    log_error "Assets directory does not exist: $ASSETS_DIR"
    log_warn "Run ./build_android.sh to copy assets"
    exit 1
fi

# Check sprites directory
echo "--- Checking sprites/ ---"
if [ -d "$ASSETS_DIR/sprites" ]; then
    log_info "sprites/ directory exists"
    
    # Count PNG files
    PNG_COUNT=$(find "$ASSETS_DIR/sprites" -name "*.png" 2>/dev/null | wc -l)
    if [ "$PNG_COUNT" -gt 0 ]; then
        log_info "Found $PNG_COUNT PNG files in sprites/"
        
        # Show subdirectories
        echo "  Subdirectories:"
        for subdir in "$ASSETS_DIR/sprites"/*/; do
            if [ -d "$subdir" ]; then
                SUBDIR_NAME=$(basename "$subdir")
                SUBDIR_COUNT=$(find "$subdir" -name "*.png" 2>/dev/null | wc -l)
                echo "    - $SUBDIR_NAME/ ($SUBDIR_COUNT PNG files)"
            fi
        done
    else
        log_error "No PNG files found in sprites/"
        ((ERRORS++))
    fi
else
    log_error "sprites/ directory is missing"
    ((ERRORS++))
fi

echo ""

# Check asset_list.txt
echo "--- Checking asset_list.txt ---"
if [ -f "$ASSETS_DIR/asset_list.txt" ]; then
    log_info "asset_list.txt exists"
    ASSET_COUNT=$(wc -l < "$ASSETS_DIR/asset_list.txt")
    log_info "Contains $ASSET_COUNT asset entries"
    
    # Show first 5 entries as preview
    echo "  First 5 entries:"
    head -5 "$ASSETS_DIR/asset_list.txt" | while read line; do
        echo "    - $line"
    done
    if [ "$ASSET_COUNT" -gt 5 ]; then
        echo "    ... and $((ASSET_COUNT - 5)) more"
    fi
else
    log_error "asset_list.txt is missing"
    log_warn "The game needs this file to locate assets on Android"
    ((ERRORS++))
fi

echo ""

# Check maps directory (optional but recommended)
echo "--- Checking maps/ (optional) ---"
if [ -d "$ASSETS_DIR/maps" ]; then
    log_info "maps/ directory exists"
    MAP_COUNT=$(find "$ASSETS_DIR/maps" -type f 2>/dev/null | wc -l)
    log_info "Found $MAP_COUNT files in maps/"
else
    log_warn "maps/ directory is missing (may be optional)"
fi

echo ""

# Compare with project root assets
echo "--- Comparing with project root ---"
if [ -d "$PROJECT_ROOT/sprites" ]; then
    ROOT_PNG_COUNT=$(find "$PROJECT_ROOT/sprites" -name "*.png" 2>/dev/null | wc -l)
    ASSETS_PNG_COUNT=$(find "$ASSETS_DIR/sprites" -name "*.png" 2>/dev/null | wc -l)
    
    if [ "$ROOT_PNG_COUNT" -eq "$ASSETS_PNG_COUNT" ]; then
        log_info "PNG count matches: $ROOT_PNG_COUNT files"
    else
        log_warn "PNG count mismatch: root=$ROOT_PNG_COUNT, assets=$ASSETS_PNG_COUNT"
    fi
fi

echo ""
echo "============================================"

if [ "$ERRORS" -eq 0 ]; then
    log_info "All required assets verified successfully!"
    echo ""
    echo "If the app still shows a gray screen, check logcat for errors:"
    echo "  adb logcat | grep -E '(SDLGame|SDL|libSDL)'"
    exit 0
else
    log_error "$ERRORS verification error(s) found!"
    echo ""
    echo "To fix missing assets, run:"
    echo "  ./build_android.sh"
    echo ""
    echo "Or manually copy assets:"
    echo "  mkdir -p $ASSETS_DIR"
    echo "  cp -r $PROJECT_ROOT/sprites $ASSETS_DIR/"
    echo "  cp $PROJECT_ROOT/asset_list.txt $ASSETS_DIR/"
    exit 1
fi
