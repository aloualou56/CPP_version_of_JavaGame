# CPP Version of JavaGame

A top-down 2D pixel-art action game written in C++17 with [SDL3](https://libsdl.org) — a port of
[JavaGame](https://github.com/aloualou56/JavaGame). The same code runs on **Windows** and on
**Android** (touch controls, landscape, native resolution).

## 📋 Contents

- [Features](#-features)
- [Controls](#-controls)
- [Project layout](#-project-layout)
- [Building on Windows](#-building-on-windows)
- [Building for Android](#-building-for-android)
- [How it works](#-how-it-works)
- [Troubleshooting](#-troubleshooting)
- [Recent changes](#-recent-changes)
- [Known issues](#-known-issues)
- [Ελληνικά](#-ελληνικά)

---

## 🎮 Features

- **Character select** — play as the boy or the girl.
- **50×50 tile world** (96 px tiles) loaded from `maps/map01.txt`, using the same 35 tile IDs and
  collision flags as the Java game's `TileManager`. The camera follows the player and stays inside the map.
- **Random scenery every run** — grass tufts, bushes and rocks scattered across the map.
- **Enemies** — 6 slimes wander around, chase you once you are within 4 tiles and hurt you on contact.
- **NPCs** — 3 friendly, blue-tinted villagers that wander around.
- **Melee combat** — knockback, a short invulnerability window after each hit and health bars on
  damaged enemies.
- **Hearts HUD** (5 hearts, drawn in quarters). When you die the screen turns red and you respawn at
  the start point.
- **Key & door** — pick up the key to open the locked door. The chest and boots are decorative,
  just like in the Java original.
- Idle / walk / attack / death animations, dust particles while walking, and depth sorting by
  "feet" position so characters walk in front of and behind objects correctly.
- **Pixel-perfect collision** with scenery, plus solid map tiles (walls, water, roads).
- **Debug view** (Tab) showing collision boxes (red) and pixel-mask bounds (green).

## 🕹️ Controls

| Action           | Desktop                               | Android                                  |
|------------------|---------------------------------------|------------------------------------------|
| Choose character | `1` = boy, `2` = girl                 | Tap the boy (left) or the girl (right)   |
| Move             | `W` `A` `S` `D`                       | Touch and drag on the left half (joystick) |
| Attack           | `R` or left mouse button (hold to keep swinging) | Touch the right half of the screen |
| Debug view       | `Tab`                                 | —                                        |
| Quit             | `Esc`                                 | —                                        |

## 📁 Project layout

```
src/, include/              Game loop, Map, Camera, TextureManager, HUD, collision …
src/ECS/, include/ECS/      Entity-component system and all components
sprites/                    Art: characters, objects, tilesets, ui, particles
maps/map01.txt              50×50 grid of tile IDs
assets/anchor_overrides.txt Manual "feet" rows used for depth sorting
android/                    Gradle project; native code in android/app/src/main/cpp/
                            (CMakeLists.txt, android_main.cpp and the SDL3 / SDL3_image AARs)
.github/workflows/          GitHub Actions workflow that builds the APK
Makefile                    Windows build (MinGW-w64)
```

---

## 🪟 Building on Windows

**Requirements**

- MinGW-w64 `g++` and `mingw32-make` (for example from [MSYS2](https://www.msys2.org/):
  `pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make`).
- The SDL3 and SDL3_image **MinGW development packages** from the
  [SDL](https://github.com/libsdl-org/SDL/releases) and
  [SDL_image](https://github.com/libsdl-org/SDL_image/releases) release pages. Copy the contents of
  each package's `x86_64-w64-mingw32` folder to `C:/SDL3` and `C:/SDL3_image`, so that
  `C:/SDL3/include`, `C:/SDL3/lib`, `C:/SDL3_image/include` and `C:/SDL3_image/lib` exist.
  (Other locations work too — change the paths at the top of the `Makefile`.)

**Build and run** (from the repository root):

```bash
mingw32-make
./Game.exe
```

The game loads `sprites/`, `maps/` and `assets/` relative to the current folder, so start it from the
repository root. `SDL3.dll` and `SDL3_image.dll` are already in the repository root. A MinGW build
also needs `libgcc_s_seh-1.dll`, `libstdc++-6.dll` and `libwinpthread-1.dll` — keep
`C:\msys64\mingw64\bin` on your `PATH`, or copy those three DLLs next to `Game.exe` when you share
the game. (`libpng.dll` and `zlib1.dll` are left over from the SDL2 version and are no longer needed.)

`mingw32-make clean` removes `obj/` and `Game.exe` (run it from Git Bash or an MSYS2 shell, because
it uses `rm`).

---

## 📱 Building for Android

The Android app is the same C++ code built as a native library (`libgame.so`) with CMake and loaded
by SDL's Java activity. It targets `armeabi-v7a` and `arm64-v8a`, minimum Android 5.0 (API 21).

**Requirements**

- **JDK 17** — set `JAVA_HOME` (for example
  `C:\Program Files\Eclipse Adoptium\jdk-17.0.6.10-hotspot`).
- **Android SDK** with platform **34** — set `ANDROID_HOME`, or put `sdk.dir=...` in
  `android/local.properties`.
- **NDK 23.1.7779620** and **CMake 3.22.1** — Gradle downloads both automatically on the first
  build (about 1 GB), as long as the SDK licenses are accepted (`sdkmanager --licenses`).
- Gradle 8.12 and the Android Gradle Plugin 8.7.3 come with the wrapper.
- SDL3 3.2.26 and SDL3_image 3.2.4 for Android are already committed under
  `android/app/src/main/cpp/SDL3*`. Nothing else to download.

**1. Copy the game assets into the Android project.** Gradle packages
`android/app/src/main/assets/` into the APK. That folder is git-ignored, so fill it before every
build. From the repository root (Git Bash on Windows, or any Linux/macOS shell):

```bash
mkdir -p android/app/src/main/assets
cp -r sprites assets maps android/app/src/main/assets/
find sprites assets maps -name '*.png' | sort > android/app/src/main/assets/asset_list.txt
```

If you delete or rename images, delete `android/app/src/main/assets/` first so no stale copies stay
behind.

**2. Build and install:**

```bash
cd android
./gradlew assembleDebug
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

On Windows `cmd`/PowerShell, use `gradlew.bat assembleDebug`. The APK only contains ARM code, but it
also runs on the standard x86_64 emulator images for Android 11 (API 30) and newer, because those
images translate ARM code.

**3. Logs:**

```bash
adb logcat -s SDL SDL/APP SDLGame
```

### GitHub Actions

`.github/workflows/android.yml` runs on every push and pull request to `main`, and can also be started
by hand (Actions tab → *Android Build* → *Run workflow*). It copies the assets, builds the debug APK,
signs it and uploads it as the **Game-APK** artifact (open the run → *Artifacts*). It signs with a
throwaway debug key unless these repository secrets are set: `KEYSTORE_BASE64`,
`KEYSTORE_PASSWORD`, `KEY_ALIAS` and `KEY_PASSWORD`.

`Game.apk` and `app-debug.apk` in the repository root are old builds from November 2025. The current
workflow uploads the APK as an artifact instead of committing it.

---

## 🔧 How it works

- **ECS** — a `Manager` owns `Entity` objects, and each entity is made of components (position,
  sprite / animation, collider, combat, attack, enemy / NPC AI, pickup, inventory …). Every frame the
  drawable components are sorted by their bottom edge, so whatever is lower on the screen is drawn in
  front.
- **Textures** — `TextureManager` loads images with SDL3_image (on Android straight from the APK),
  caches them by path and switches them to nearest-neighbour scaling so the pixel art stays sharp.
- **Map** — `maps/map01.txt` holds 50 rows × 50 tile IDs (0–34). It is read through SDL's file I/O so
  the same code works from the APK. The opaque ground tile (ID 3, `sprites/tilesets/grass.png`) is
  drawn under every cell first. The other tiles — grass tufts, dirt, roads, walls, water — have
  transparent parts and are drawn on top of it.
- **Collision** — AABB checks plus pixel-perfect masks made from each sprite's alpha channel. Only
  pixels with alpha above 200 count as solid, so soft edges and shadows don't act as invisible walls.
  Map tiles flagged as solid block movement as well.
- **Depth anchors** — the "feet" row of each sprite is detected from its alpha.
  `assets/anchor_overrides.txt` overrides it for sprites where the detection is off.
- **Hearts HUD** — loads `sprites/ui/heart_*.png`. If an image is missing or can't be read, the HUD
  draws the heart itself and (on desktop) saves it back to `sprites/ui/` with the small built-in PNG
  writer in `src/stb_image_write.h`. Despite its name, that file is not the real stb library.

---

## 🩺 Troubleshooting

**Android shows only a gray or black screen** — the images were not packaged. Make sure
`android/app/src/main/assets/sprites/` exists before you build (step 1 above). Check the APK with
`unzip -l app/build/outputs/apk/debug/app-debug.apk | grep assets/sprites`, and look for
`Failed to load image` in logcat.

**The ground shows the wrong color on one platform** — the ground must come from an opaque tile.
The game clears every frame to black, so if you see black ground, check logcat (or the console)
for `Map loaded` and confirm that the ground tile (ID 3) image is fully opaque.

**`Corrupt PNG` in the log** — the image file itself is broken (open it in an image viewer to check).
SDL_image refuses invalid PNGs on every platform.

**A change to a header has no effect on Windows** — the `Makefile` only tracks `.cpp` files, so
editing a header doesn't rebuild the files that include it. Delete the `obj` folder (or run
`mingw32-make clean`) and build again.

---

## 📝 Recent changes

- **October 2026 — Android "red grass" fixed.** Since November 2025, `grass1–4.png` are
  transparent grass tufts (they are also used as decorations), but the map still drew them as ground
  with nothing underneath. On Android the map file never loaded at all — it was read with
  `std::ifstream`, which can't see files inside the APK — so every cell fell back to the transparent
  tile 0. The screen was also never cleared to a fixed color, so whatever was drawn last showed
  through: the loading bar's green on PC and the red attack button on Android. Changing pixel formats
  could never fix that. Now the map is read through SDL's I/O, the opaque ground tile is drawn under
  every cell, and each frame starts from black.
- **October 2026 — hearts are red again (all platforms).** The heart images in `sprites/ui/` were
  invalid PNGs, because the built-in PNG writer stored chunk lengths and CRCs in the wrong byte order
  and computed the CRCs incorrectly. SDL_image rejected them ("Corrupt PNG"), so the HUD drew its own
  hearts. Those came out blue, because the red and blue channels were swapped. Fixed the writer and
  the channel order, and regenerated the five heart images.
- **October 2026 — Android build fixes.** Removed the SDL2-only `SDL_HINT_RENDER_SCALE_QUALITY`
  hint, which doesn't compile against SDL3; added the newer source files (combat, AI, pickups, seven
  segment digits) to the Android CMake build; the character-select screen can now be tapped.
- **November 2025** — Android port with the GitHub Actions build; migration from SDL2 to SDL3; fixed
  invisible walls around bushes by raising the collision alpha threshold from 128 to 200.

## ⚠️ Known issues

- On Android, SDL also reports every touch as a left mouse click, and `AttackComponent` treats a
  held left button as "attack". So the player keeps swinging while you hold the joystick.
- Vibration: nothing triggers a rumble anymore, and the haptic setup in `Game::init` still checks
  SDL2-style return values (SDL3 returns `true` on success), so the device is never opened.
- `Makefile.linux` (cross-compiling the Windows build from Linux) and `.gitignore` still contain
  unresolved merge-conflict markers, so `Makefile.linux` can't be used as it is.

---

## 🇬🇷 Ελληνικά

Παιχνίδι δράσης 2D με pixel art, γραμμένο σε C++17 με SDL3 — μεταφορά του
[JavaGame](https://github.com/aloualou56/JavaGame). Τρέχει σε Windows και Android.

**Χειρισμός:** `1` / `2` (ή άγγιγμα στο Android) για επιλογή χαρακτήρα, `WASD` για κίνηση, `R` ή
αριστερό κλικ για επίθεση, `Tab` για debug, `Esc` για έξοδο. Στο Android: joystick στο αριστερό
μισό της οθόνης, επίθεση στο δεξί.

**Μεταγλώττιση:** στα Windows `mingw32-make` (SDL3 στο `C:/SDL3`, SDL3_image στο `C:/SDL3_image`).
Για Android: αντιγράψτε τα `sprites/`, `assets/` και `maps/` στο `android/app/src/main/assets/` και
τρέξτε `gradlew assembleDebug` μέσα στον φάκελο `android/` (λεπτομέρειες παραπάνω).

**Διόρθωση «κόκκινου γρασιδιού» (Οκτώβριος 2026):** τα `grass1–4.png` είναι διάφανες τούφες
χόρτου, αλλά ο χάρτης τα σχεδίαζε ως έδαφος χωρίς τίποτα από κάτω, και στο Android ο χάρτης δεν
φορτωνόταν καθόλου (το `std::ifstream` δεν βλέπει τα αρχεία μέσα στο APK). Έτσι φαινόταν ό,τι είχε
σχεδιαστεί τελευταίο: πράσινο στο PC, κόκκινο (από το κουμπί επίθεσης) στο Android. Τώρα ο χάρτης
φορτώνεται μέσω SDL, κάτω από κάθε πλακίδιο σχεδιάζεται αδιαφανές έδαφος και κάθε frame ξεκινά από
μαύρο. Επίσης οι καρδιές της ζωής ήταν μπλε: τα PNG τους ήταν χαλασμένα (λάθος σειρά byte στον
ενσωματωμένο PNG writer) και το κόκκινο με το μπλε είχαν αντιστραφεί. Διορθώθηκαν και τα δύο, οπότε οι
καρδιές είναι ξανά κόκκινες.

---

## 🙏 Credits

- Game and code: [aloualou56](https://github.com/aloualou56), ported from
  [JavaGame](https://github.com/aloualou56/JavaGame).
- Libraries: [SDL3](https://github.com/libsdl-org/SDL) and
  [SDL3_image](https://github.com/libsdl-org/SDL_image).

## 📄 License

There is no LICENSE file in the repository yet.
