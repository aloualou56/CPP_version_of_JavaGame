# CPP Version of JavaGame

A 2D game built with C++ and SDL2, featuring pixel-perfect collision detection and sprite-based graphics.

## 🎮 Game Controls

- **W, A, S, D** - Move character
- **R** - Attack animation
- **Tab** - Toggle debug mode (shows collision boxes)

## 📋 Table of Contents

- [Recent Updates](#recent-updates)
- [Collision Bug Fix](#collision-bug-fix)
- [Visual Guide](#visual-guide)
- [Building the Game](#building-the-game)
- [Android (APK) Build](#android-apk-build)
- [Distribution](#distribution)
- [Technical Details](#technical-details)

---

## 🆕 Recent Updates

### Version 1.1 (2025-11-24)

**Bug Fixes:**
- ✅ Fixed collision detection with bushes (thamnos_tonia) - removed "invisible wall" effect
- ✅ Adjusted alpha threshold from 128 to 200 for pixel-perfect collision
- ✅ Added fallback mask loading for bush entities

**Files Modified:**
- `src/ImageMask.hpp` - Alpha threshold adjustment (lines 42, 74)
- `src/ECS/ColliderComponent.hpp` - Bush fallback mask loading (lines 58-59)
- `src/ECS/PositionComponent.cpp` - Clarifying comments

**New Game Build:**
- `Game.exe` - Recompiled with fixes (5.3 MB)
- Updated SDL2.dll to version 2.30.9
- Updated SDL2_image.dll to version 2.8.2

---

## 🐛 Collision Bug Fix

### Problem Description

The game had a collision detection bug where:
- **Rocks**: Collision worked correctly - you could get close to the visual sprite before collision occurred ✓
- **Bushes (thamnos_tonia)**: There was an "invisible wall" - collision occurred before touching the visible sprite ❌

### Root Cause

The bug was caused by the alpha threshold used in pixel-perfect collision detection. The code in `src/ImageMask.hpp` was treating any pixel with alpha > 128 as solid for collision purposes.

When sprite images are created with anti-aliasing, shadows, or glow effects, they often have semi-transparent pixels around the edges. These pixels have alpha values between 129 and 254 (not fully opaque, but not fully transparent either).

**With the original threshold of 128:**
- Pixels with alpha 0-128: Treated as transparent (no collision)
- Pixels with alpha 129-255: Treated as solid (collision occurs) ⚠️

This meant that sprites with visual effects like soft edges or glows would have collision detection that extended beyond the visible sprite, creating an "invisible wall" effect.

### Solution

The fix involved two changes:

#### 1. Increased Alpha Threshold (src/ImageMask.hpp)

Changed the alpha threshold from **128 to 200**:

```cpp
// OLD CODE (line 42):
if (a > 128) {
    // pixel considered solid
}

// NEW CODE:
if (a > 200) {  // Use higher threshold to ignore semi-transparent pixels
    // pixel considered solid
}
```

This change was applied in two places:
1. When finding the bounding box of solid pixels (line 42)
2. When creating the pixel mask array (line 74)

**Effect**: Only nearly-opaque pixels (alpha > 200) are now considered solid. Semi-transparent pixels used for anti-aliasing and visual effects (alpha 129-200) are now ignored for collision purposes.

#### 2. Added Fallback Mask Loading (src/ECS/ColliderComponent.hpp)

Added a fallback case for bushes in the mask loading code:

```cpp
// Added to line 58-59:
} else if (tag == "bush") {
   mask.loadFromPNG("sprites/objects/thamnos_tonia.png");
```

**Effect**: Ensures that even if the mask fails to load from the SpriteComponent for some reason, bushes will still get their collision mask loaded from the sprite file.

### Testing the Fix

To verify the fix works:

1. Run the game with debug mode enabled (press **Tab** key)
2. Approach a rock - you should see the red collision box and green mask box align closely with the visible sprite
3. Approach a bush - the collision should now align with the visible sprite instead of having an invisible wall
4. You should be able to walk close to bushes without hitting an invisible barrier

---

## 📊 Visual Guide

### Before the Fix

```
Bush Sprite with Semi-Transparent Glow:
┌─────────────────────────────────┐
│         (invisible)              │ ← Alpha 50-100 (very transparent glow)
│    ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒         │ ← Alpha 150 (semi-transparent edge)
│   ▒████████████████████▒        │ ← Alpha 220 (nearly opaque bush)
│  ▒██████████████████████▒       │
│  ▒██████████████████████▒       │ ← Visible bush
│  ▒██████████████████████▒       │
│   ▒████████████████████▒        │
│    ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒         │
│         (invisible)              │
└─────────────────────────────────┘

OLD Collision Detection (Alpha > 128):
┌─────────────────────────────────┐
│         XXXXXXXXXX               │ ← COLLISION DETECTED HERE!
│    XXXXXXXXXXXXXXXXXXXX          │ ← Semi-transparent pixels cause collision
│   XXXXXXXXXXXXXXXXXXXXXXX        │
│  XXXXXXXXXXXXXXXXXXXXXXXXX       │
│  XXXXXXXXXXXXXXXXXXXXXXXXX       │ ← Player hits invisible wall here
│  XXXXXXXXXXXXXXXXXXXXXXXXX       │    before touching visible sprite
│   XXXXXXXXXXXXXXXXXXXXXXX        │
│    XXXXXXXXXXXXXXXXXXXX          │
│         XXXXXXXXXX               │
└─────────────────────────────────┘
       ↑
   Invisible Wall! Player blocked too early
```

### After the Fix

```
NEW Collision Detection (Alpha > 200):
┌─────────────────────────────────┐
│         (no collision)           │ ← No collision in glow area
│    (no collision)                │ ← Semi-transparent edge ignored
│   ████████████████████           │ ← Collision only on solid bush
│  ██████████████████████          │
│  ██████████████████████          │ ← Player can get close to sprite
│  ██████████████████████          │    before collision
│   ████████████████████           │
│    (no collision)                │
│         (no collision)           │
└─────────────────────────────────┘
       ↑
   Natural collision! Player can approach visual sprite
```

### Alpha Threshold Comparison

```
Pixel Alpha Values:
0  ─────────────────────────── Fully Transparent
50  ← Glow/Shadow effects
100 
128 ← OLD threshold (too sensitive)
150 ← Anti-aliasing edges
200 ← NEW threshold (just right!)
220 
255 ─────────────────────────── Fully Opaque

Effect of Change:
- Alpha 0-128:   Always transparent (no collision)     [UNCHANGED]
- Alpha 129-200: NOW transparent (no collision)        [CHANGED - This fixes the bug!]
- Alpha 201-255: Solid (collision detected)            [UNCHANGED]
```

### Game Behavior Comparison

**Before Fix (Bushes with invisible wall):**
```
Player Movement:

 🚶              🌳
  →  →  →  ⚠️ BLOCKED!
           ↑
      Invisible wall - Can't reach the bush
```

**After Fix (Bushes work correctly):**
```
Player Movement:

 🚶              🌳
  →  →  →  →  😊 Touch!
                ↑
           Natural collision - Can reach the visible sprite
```

---

## 🔨 Building the Game

### For Windows Users (Native Compilation)

If you have a Windows development environment set up:

**Prerequisites:**
- MinGW-w64 compiler (g++)
- SDL2 development libraries (place in `C:/SDL2/`)
- SDL2_image development libraries

**Build:**
```cmd
make clean
make
```

**Run:**
```cmd
Game.exe
```

### For Linux Users (Cross-Compilation)

The provided `Game.exe` was cross-compiled on Linux using MinGW.

**1. Install MinGW cross-compiler:**
```bash
sudo apt-get update
sudo apt-get install mingw-w64
```

**2. Download SDL2 libraries:**
```bash
cd /tmp
wget https://github.com/libsdl-org/SDL/releases/download/release-2.30.9/SDL2-devel-2.30.9-mingw.tar.gz
tar -xzf SDL2-devel-2.30.9-mingw.tar.gz

wget https://github.com/libsdl-org/SDL_image/releases/download/release-2.8.2/SDL2_image-devel-2.8.2-mingw.tar.gz
tar -xzf SDL2_image-2.8.2-mingw.tar.gz
```

**3. Build:**
```bash
make -f Makefile.linux clean
make -f Makefile.linux
```

**4. Copy DLLs:**
```bash
cp /tmp/SDL2-2.30.9/x86_64-w64-mingw32/bin/SDL2.dll .
cp /tmp/SDL2_image-2.8.2/x86_64-w64-mingw32/bin/*.dll .
```

**5. Transfer to Windows:**
- Copy `Game.exe` and all `*.dll` files to a Windows machine
- Ensure the `maps/` and `sprites/` directories are present
- Run `Game.exe`

---

## 📱 Android (APK) Build

This game can be built as an Android APK using the provided Android project scaffolding. The build uses SDL2 for Android and produces APKs for both `armeabi-v7a` and `arm64-v8a` architectures.

### Prerequisites

- **Android SDK** with API level 34
- **Android NDK** r23.1.7779620
- **CMake** 3.22.1 (installed via Android SDK)
- **Java JDK** 17
- **Gradle** 8.0 (wrapper included)

### Build Locally with Android Studio

1. **Install Android Studio** from [developer.android.com](https://developer.android.com/studio)

2. **Open the project:**
   - Launch Android Studio
   - Select "Open an existing project"
   - Navigate to and select the `android/` directory

3. **Configure NDK:**
   - Go to **File → Project Structure → SDK Location**
   - Set Android NDK location (or let Android Studio download it)
   - Required NDK version: `23.1.7779620`

4. **Sync Gradle:**
   - Click "Sync Project with Gradle Files" in the toolbar
   - Wait for the sync to complete (SDL2 will be downloaded automatically)

5. **Build the APK:**
   - **Debug APK:** Run → Run 'app' (or press Shift+F10)
   - **Release APK:** Build → Generate Signed Bundle / APK
     - Select APK
     - Create or use existing keystore
     - Select release build variant
     - APK will be in `android/app/build/outputs/apk/release/`

### Build Locally from Command Line

The `android/build_android.sh` script automates the command-line build process:

```bash
# Navigate to the android directory
cd android

# Make the build script executable
chmod +x build_android.sh

# Build a release APK (unsigned)
./build_android.sh

# Build a debug APK
./build_android.sh --debug

# Clean and rebuild
./build_android.sh --clean

# Build and sign the APK (requires keystore configuration)
./build_android.sh --sign
```

**Manual Gradle build:**

```bash
cd android

# Download SDL2 (first time only)
./build_android.sh  # This downloads SDL2 automatically

# Or use Gradle directly
./gradlew assembleRelease

# The APK will be at:
# android/app/build/outputs/apk/release/app-release-unsigned.apk
```

**Environment Variables:**

| Variable | Description |
|----------|-------------|
| `ANDROID_SDK_ROOT` | Path to Android SDK (auto-detected from common locations) |
| `ANDROID_NDK_HOME` | Path to Android NDK (optional) |
| `KEYSTORE_FILE` | Path to keystore for signing |
| `KEYSTORE_PASSWORD` | Keystore password |
| `KEY_ALIAS` | Key alias in keystore |
| `KEY_PASSWORD` | Key password |

### Build in CI (GitHub Actions)

The repository includes a GitHub Actions workflow (`.github/workflows/android.yml`) that automatically builds the APK on every push to `main`/`master` or on pull requests.

**Workflow Features:**
- Builds for `armeabi-v7a` and `arm64-v8a`
- Caches Gradle and SDL2 for faster builds
- Uploads unsigned APK as artifact
- Optionally signs APK if secrets are configured

**To download the APK:**
1. Go to the **Actions** tab in GitHub
2. Click on the latest successful workflow run
3. Download the `app-release-unsigned` artifact

**To enable APK signing in CI:**

1. **Create a keystore** (if you don't have one):
   ```bash
   keytool -genkey -v -keystore my-release-key.jks \
     -keyalg RSA -keysize 2048 -validity 10000 \
     -alias my-key-alias
   ```

2. **Encode keystore to base64:**
   ```bash
   base64 -i my-release-key.jks > keystore-base64.txt
   ```

3. **Add secrets to GitHub:**
   - Go to your repository → Settings → Secrets and variables → Actions
   - Add the following secrets:
     - `KEYSTORE_BASE64`: Contents of `keystore-base64.txt`
     - `KEYSTORE_PASSWORD`: Your keystore password
     - `KEY_ALIAS`: Your key alias (e.g., `my-key-alias`)
     - `KEY_PASSWORD`: Your key password

4. **Delete local keystore files** (never commit them!):
   ```bash
   rm my-release-key.jks keystore-base64.txt
   ```

### Troubleshooting

#### Common Issues

| Issue | Solution |
|-------|----------|
| **NDK not found** | Install via SDK Manager: `sdkmanager --install "ndk;23.1.7779620"` |
| **CMake not found** | Install via SDK Manager: `sdkmanager --install "cmake;3.22.1"` |
| **SDL2 download fails** | Check internet connection; manually download from [SDL2 releases](https://github.com/libsdl-org/SDL/releases) |
| **Build fails with "ABI not found"** | Ensure NDK is installed for required ABIs |
| **App crashes on startup** | Check logcat for errors; verify all assets are packaged |
| **Audio not working** | SDL2 audio requires proper Android permissions and initialization |
| **Touch input issues** | Verify SDL event handling for Android touch events |

#### Checking Logs

```bash
# Connect device and view logs
adb logcat | grep -E "(SDLGame|SDL|libSDL)"

# Or in Android Studio: View → Tool Windows → Logcat
```

#### Asset Packaging

Game assets (maps, sprites) are packaged from the root directories. If assets are missing:

1. Verify `android/app/build.gradle` has correct asset paths:
   ```gradle
   sourceSets {
       main {
           assets.srcDirs = ['src/main/assets', '../../maps', '../../sprites']
       }
   }
   ```

2. Check that asset files exist and are not gitignored

---

## 📦 Distribution

### Required Files

When distributing the game, include:
- `Game.exe` - The main executable (~5.3 MB)
- `SDL2.dll` - SDL2 runtime library (~2.2 MB)
- `SDL2_image.dll` - SDL2_image runtime library (~1.1 MB)
- `libpng.dll` - PNG image support (~171 KB)
- `zlib1.dll` - Compression library (~87 KB)
- `maps/` - Directory containing map files
- `sprites/` - Directory containing all game sprites

### Directory Structure

```
Game/
├── Game.exe
├── SDL2.dll
├── SDL2_image.dll
├── libpng.dll
├── zlib1.dll
├── maps/
│   └── map01.txt
└── sprites/
    ├── characters/
    ├── objects/
    └── tilesets/
```

---

## 🔧 Technical Details

### Pixel-Perfect Collision Detection

The game uses a sophisticated pixel-perfect collision system that:

1. **Loads sprite images** and analyzes each pixel's alpha channel
2. **Creates a binary mask** of "solid" vs "transparent" pixels
3. **Crops the mask** to the minimal bounding box of solid pixels
4. **During collision detection**, checks if solid pixels overlap between two sprites

### Alpha Channel Values

- **Alpha 0**: Fully transparent (invisible)
- **Alpha 1-127**: Very transparent to semi-transparent
- **Alpha 128-200**: Semi-transparent (visible but somewhat transparent)
- **Alpha 201-255**: Nearly opaque to fully opaque

### Why The Threshold Matters

Different sprite creation tools and artists use different techniques:
- **Simple sprites** might only use alpha 0 (transparent) and 255 (opaque)
- **Anti-aliased sprites** use gradual alpha transitions at edges (e.g., 180, 200, 220, 240, 255)
- **Sprites with glows or shadows** have larger semi-transparent areas

The threshold determines which pixels should cause collision:
- **Too low (like 128)**: Creates invisible walls around soft-edged sprites ❌
- **Too high (like 254)**: Might allow players to walk through visible parts of sprites ❌
- **Just right (200)**: Provides a good balance ✓

**The value of 200 provides the optimal balance:**
- Fully opaque pixels (alpha 201-255) cause collision ✓
- Anti-aliasing edges (alpha 150-200) don't cause collision ✓
- Very transparent effects (alpha 1-150) don't cause collision ✓

### Implementation Details

**Code Changes in ImageMask.hpp:**

**Line 42 - Bounding Box Detection:**
```cpp
// OLD:
if (a > 128) {
    // Marks pixel as part of collision mask
}

// NEW:
if (a > 200) {  // Χρησιμοποιεί υψηλότερο κατώφλι για να αγνοεί ημιδιαφανή pixels
    // Only nearly-opaque pixels are part of collision mask
}
```

**Line 74 - Mask Creation:**
```cpp
// OLD:
mask[y][x] = (a > 128);

// NEW:
mask[y][x] = (a > 200);  // Χρησιμοποιεί το ίδιο κατώφλι με την εύρεση περιγράμματος
```

**Why This Works:**

1. Sprite artists often add soft edges, glows, or shadows using semi-transparent pixels
2. Old threshold (128) treated anything more than 50% transparent as solid
3. New threshold (200) requires pixels to be nearly opaque (78%+ opacity) to be solid
4. Result: Visual effects don't interfere with collision, only the main sprite does

---

## 🔮 Future Improvements

Potential enhancements for the collision system:

1. **Per-sprite threshold**: Allow different sprites to specify their own alpha thresholds
2. **Configuration file**: Make the threshold configurable without recompiling
3. **Visual editor**: Create a tool to visualize and adjust collision masks
4. **Performance optimization**: Cache the pixel-perfect checks or use spatial hashing

---

## 📝 Credits

- **Original Game**: JavaGame (converted to C++)
- **Bug Report**: aloualou56
- **Bug Fix**: GitHub Copilot
- **Date**: November 24, 2025
- **Libraries**: SDL2, SDL2_image

---

## 📄 License

See LICENSE file for details.

---

# Ελληνικά (Greek)

## 🐛 Διόρθωση Σφάλματος Σύγκρουσης

### Περιγραφή Προβλήματος

Το παιχνίδι είχε ένα σφάλμα στον εντοπισμό συγκρούσεων όπου:
- **Βράχοι**: Η σύγκρουση λειτουργούσε σωστά - μπορούσες να πλησιάσεις το ορατό sprite πριν συμβεί σύγκρουση ✓
- **Θάμνοι (thamnos_tonia)**: Υπήρχε ένας "αόρατος τοίχος" - η σύγκρουση συνέβαινε πριν ακουμπήσεις το ορατό sprite ❌

### Αιτία Προβλήματος

Το σφάλμα προκλήθηκε από το κατώφλι alpha που χρησιμοποιείται στον εντοπισμό συγκρούσεων pixel-perfect. Ο κώδικας στο `src/ImageMask.hpp` θεωρούσε οποιοδήποτε pixel με alpha > 128 ως στερεό για σκοπούς σύγκρουσης.

Όταν οι εικόνες sprite δημιουργούνται με anti-aliasing, σκιές ή εφέ λάμψης, συχνά έχουν ημιδιαφανή pixels γύρω από τις άκρες. Αυτά τα pixels έχουν τιμές alpha μεταξύ 129 και 254 (όχι πλήρως αδιαφανή, αλλά όχι πλήρως διαφανή).

### Λύση

Η διόρθωση περιελάμβανε δύο αλλαγές:

**1. Αύξηση Κατωφλίου Alpha από 128 σε 200**
- Μόνο τα σχεδόν αδιαφανή pixels (alpha > 200) θεωρούνται τώρα στερεά
- Τα ημιδιαφανή pixels που χρησιμοποιούνται για anti-aliasing και οπτικά εφέ (alpha 129-200) αγνοούνται τώρα

**2. Προστέθηκε Εφεδρική Φόρτωση Μάσκας για τους θάμνους**
- Διασφαλίζει ότι οι θάμνοι πάντα φορτώνουν τη μάσκα σύγκρουσής τους

### Δοκιμή

1. Εκτελέστε το παιχνίδι με ενεργοποιημένη τη λειτουργία debug (πατήστε **Tab**)
2. Πλησιάστε έναν βράχο - θα δείτε το κόκκινο πλαίσιο σύγκρουσης να ευθυγραμμίζεται με το ορατό sprite
3. Πλησιάστε έναν θάμνο - η σύγκρουση τώρα ευθυγραμμίζεται με το ορατό sprite
4. Μπορείτε να περπατήσετε κοντά σε θάμνους χωρίς να χτυπήσετε αόρατο εμπόδιο

## 🔨 Οδηγίες Κατασκευής

### Για Χρήστες Windows

```cmd
make clean
make
Game.exe
```

### Για Χρήστες Linux (Διασταυρούμενη Μεταγλώττιση)

```bash
sudo apt-get install mingw-w64
make -f Makefile.linux
```

## 📦 Απαιτούμενα Αρχεία για Διανομή

- Game.exe (~5.3 MB)
- SDL2.dll (~2.2 MB)
- SDL2_image.dll (~1.1 MB)
- libpng.dll (~171 KB)
- zlib1.dll (~87 KB)
- Κατάλογος maps/
- Κατάλογος sprites/

---

**Ημερομηνία Ενημέρωσης**: 24 Νοεμβρίου 2025
