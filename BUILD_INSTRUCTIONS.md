# Build Instructions / Οδηγίες Κατασκευής

## English

### For Windows Users (Native Compilation)

If you have a Windows development environment set up:

1. **Prerequisites:**
   - MinGW-w64 compiler (g++)
   - SDL2 development libraries (place in `C:/SDL2/`)
   - SDL2_image development libraries

2. **Build:**
   ```cmd
   make clean
   make
   ```

3. **Run:**
   ```cmd
   Game.exe
   ```

### For Linux Users (Cross-Compilation)

The provided `Game.exe` was cross-compiled on Linux using MinGW. If you want to rebuild it:

1. **Install MinGW cross-compiler:**
   ```bash
   sudo apt-get update
   sudo apt-get install mingw-w64
   ```

2. **Download SDL2 libraries:**
   ```bash
   cd /tmp
   wget https://github.com/libsdl-org/SDL/releases/download/release-2.30.9/SDL2-devel-2.30.9-mingw.tar.gz
   tar -xzf SDL2-devel-2.30.9-mingw.tar.gz
   
   wget https://github.com/libsdl-org/SDL_image/releases/download/release-2.8.2/SDL2_image-devel-2.8.2-mingw.tar.gz
   tar -xzf SDL2_image-2.8.2-mingw.tar.gz
   ```

3. **Update Makefile.linux paths if needed** (currently set to /tmp/SDL2-*)

4. **Build:**
   ```bash
   make -f Makefile.linux clean
   make -f Makefile.linux
   ```

5. **Copy DLLs:**
   ```bash
   cp /tmp/SDL2-2.30.9/x86_64-w64-mingw32/bin/SDL2.dll .
   cp /tmp/SDL2_image-2.8.2/x86_64-w64-mingw32/bin/*.dll .
   ```

6. **Transfer to Windows:**
   - Copy `Game.exe` and all `*.dll` files to a Windows machine
   - Ensure the `maps/` and `sprites/` directories are present
   - Run `Game.exe`

### Required Files for Distribution

When distributing the game, include:
- `Game.exe` - The main executable
- `SDL2.dll` - SDL2 runtime library
- `SDL2_image.dll` - SDL2_image runtime library  
- `libpng.dll` - PNG image support
- `zlib1.dll` - Compression library
- `maps/` - Directory containing map files
- `sprites/` - Directory containing all game sprites

### File Sizes (Approximate)
- Game.exe: ~5.3 MB
- SDL2.dll: ~2.2 MB
- SDL2_image.dll: ~1.1 MB
- libpng.dll: ~171 KB
- zlib1.dll: ~87 KB

---

## Ελληνικά (Greek)

### Για Χρήστες Windows (Εγγενής Μεταγλώττιση)

Αν έχετε περιβάλλον ανάπτυξης Windows:

1. **Προαπαιτούμενα:**
   - Μεταγλωττιστής MinGW-w64 (g++)
   - Βιβλιοθήκες ανάπτυξης SDL2 (τοποθετήστε στο `C:/SDL2/`)
   - Βιβλιοθήκες ανάπτυξης SDL2_image

2. **Κατασκευή:**
   ```cmd
   make clean
   make
   ```

3. **Εκτέλεση:**
   ```cmd
   Game.exe
   ```

### Για Χρήστες Linux (Διασταυρούμενη Μεταγλώττιση)

Το παρεχόμενο `Game.exe` μεταγλωττίστηκε διασταυρωμένα σε Linux χρησιμοποιώντας MinGW. Αν θέλετε να το ξαναφτιάξετε:

1. **Εγκατάσταση μεταγλωττιστή MinGW:**
   ```bash
   sudo apt-get update
   sudo apt-get install mingw-w64
   ```

2. **Λήψη βιβλιοθηκών SDL2:**
   ```bash
   cd /tmp
   wget https://github.com/libsdl-org/SDL/releases/download/release-2.30.9/SDL2-devel-2.30.9-mingw.tar.gz
   tar -xzf SDL2-devel-2.30.9-mingw.tar.gz
   
   wget https://github.com/libsdl-org/SDL_image/releases/download/release-2.8.2/SDL2_image-devel-2.8.2-mingw.tar.gz
   tar -xzf SDL2_image-2.8.2-mingw.tar.gz
   ```

3. **Ενημέρωση διαδρομών Makefile.linux αν χρειάζεται** (επί του παρόντος ορισμένες σε /tmp/SDL2-*)

4. **Κατασκευή:**
   ```bash
   make -f Makefile.linux clean
   make -f Makefile.linux
   ```

5. **Αντιγραφή DLLs:**
   ```bash
   cp /tmp/SDL2-2.30.9/x86_64-w64-mingw32/bin/SDL2.dll .
   cp /tmp/SDL2_image-2.8.2/x86_64-w64-mingw32/bin/*.dll .
   ```

6. **Μεταφορά σε Windows:**
   - Αντιγράψτε το `Game.exe` και όλα τα αρχεία `*.dll` σε μηχάνημα Windows
   - Βεβαιωθείτε ότι οι κατάλογοι `maps/` και `sprites/` είναι παρόντες
   - Εκτελέστε το `Game.exe`

### Απαιτούμενα Αρχεία για Διανομή

Κατά τη διανομή του παιχνιδιού, συμπεριλάβετε:
- `Game.exe` - Το κύριο εκτελέσιμο
- `SDL2.dll` - Βιβλιοθήκη εκτέλεσης SDL2
- `SDL2_image.dll` - Βιβλιοθήκη εκτέλεσης SDL2_image
- `libpng.dll` - Υποστήριξη εικόνας PNG
- `zlib1.dll` - Βιβλιοθήκη συμπίεσης
- `maps/` - Κατάλογος που περιέχει αρχεία χάρτη
- `sprites/` - Κατάλογος που περιέχει όλα τα sprites του παιχνιδιού

### Μεγέθη Αρχείων (Κατά Προσέγγιση)
- Game.exe: ~5.3 MB
- SDL2.dll: ~2.2 MB
- SDL2_image.dll: ~1.1 MB
- libpng.dll: ~171 KB
- zlib1.dll: ~87 KB

---

## Changelog / Αλλαγές

### Version 1.1 (2025-11-24)

**Bug Fixes:**
- Fixed collision detection with bushes (thamnos_tonia) - removed "invisible wall" effect
- Adjusted alpha threshold from 128 to 200 for pixel-perfect collision
- Added fallback mask loading for bush entities

**Διορθώσεις Σφαλμάτων:**
- Διορθώθηκε η ανίχνευση σύγκρουσης με θάμνους (thamnos_tonia) - αφαιρέθηκε το εφέ "αόρατου τοίχου"
- Προσαρμόστηκε το κατώφλι alpha από 128 σε 200 για σύγκρουση pixel-perfect
- Προστέθηκε εφεδρική φόρτωση μάσκας για οντότητες θάμνων

**Files Modified / Αρχεία που Τροποποιήθηκαν:**
- src/ImageMask.hpp
- src/ECS/ColliderComponent.hpp
- src/ECS/PositionComponent.cpp

**Documentation Added / Προστέθηκε Τεκμηρίωση:**
- COLLISION_FIX_DOCUMENTATION.md (English)
- COLLISION_FIX_DOCUMENTATION_GREEK.md (Ελληνικά)
- VISUAL_GUIDE.md (English with diagrams)
- VISUAL_GUIDE_GREEK.md (Ελληνικά με διαγράμματα)
- BUILD_INSTRUCTIONS.md (This file / Αυτό το αρχείο)
