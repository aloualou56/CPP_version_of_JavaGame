# Windows Setup Guide for CPP_version_of_JavaGame

This guide provides detailed instructions for building and running the game on Windows using MinGW.

## Prerequisites

### MinGW Installation

You need to have MinGW (Minimalist GNU for Windows) installed on your system to compile C++ code on Windows.

**Option 1: Install MinGW-w64**
1. Download the MinGW-w64 installer from [https://www.mingw-w64.org/downloads/](https://www.mingw-w64.org/downloads/)
2. For a simple installation, use the [MSYS2](https://www.msys2.org/) installer
3. After installing MSYS2, open the MSYS2 terminal and run:
   ```bash
   pacman -S mingw-w64-x86_64-toolchain
   pacman -S make
   ```
4. Add MinGW to your system PATH:
   - Add `C:\msys64\mingw64\bin` to your system's PATH environment variable
   - Verify installation by opening Command Prompt and typing: `g++ --version`

**Option 2: Install TDM-GCC**
1. Download TDM-GCC from [https://jmeubank.github.io/tdm-gcc/](https://jmeubank.github.io/tdm-gcc/)
2. Run the installer and follow the installation wizard
3. Verify installation by opening Command Prompt and typing: `g++ --version`

## SDL2 Installation

The game requires SDL2 and SDL2_image libraries.

### Step 1: Download SDL2 Libraries

Download the following development libraries for MinGW:

1. **SDL2**:
   - Visit [https://github.com/libsdl-org/SDL/releases](https://github.com/libsdl-org/SDL/releases)
   - Download the latest `SDL2-devel-X.X.X-mingw.tar.gz` (e.g., SDL2-devel-2.28.5-mingw.tar.gz)

2. **SDL2_image**:
   - Visit [https://github.com/libsdl-org/SDL_image/releases](https://github.com/libsdl-org/SDL_image/releases)
   - Download the latest `SDL2_image-devel-X.X.X-mingw.tar.gz`

### Step 2: Extract and Organize SDL2 Files

**Recommended Directory Structure:**

Create a directory `C:\SDL2` and organize the files as follows:

```
C:\SDL2\
├── include\
│   ├── SDL2\
│   │   ├── SDL.h
│   │   ├── SDL_image.h
│   │   └── ... (other SDL2 headers)
├── lib\
│   ├── libSDL2.a
│   ├── libSDL2main.a
│   ├── libSDL2_image.a
│   └── ... (other .a files)
└── bin\
    ├── SDL2.dll
    ├── SDL2_image.dll
    └── ... (other required DLLs)
```

**Extraction Steps:**

1. Extract the downloaded `SDL2-devel-X.X.X-mingw.tar.gz`:
   - Navigate to the appropriate architecture folder (e.g., `x86_64-w64-mingw32` for 64-bit)
   - Copy the `include/SDL2` folder to `C:\SDL2\include\SDL2`
   - Copy all `.a` files from `lib` folder to `C:\SDL2\lib\`
   - Copy all `.dll` files from `bin` folder to `C:\SDL2\bin\`

2. Extract the downloaded `SDL2_image-devel-X.X.X-mingw.tar.gz`:
   - Navigate to the appropriate architecture folder
   - Copy all files from `include/SDL2` to `C:\SDL2\include\SDL2\`
   - Copy all `.a` files from `lib` to `C:\SDL2\lib\`
   - Copy all `.dll` files from `bin` to `C:\SDL2\bin\`

**Alternative Directory:**
If you prefer a different location, you can install SDL2 anywhere (e.g., `C:\dev\SDL2` or `D:\libraries\SDL2`). Just make sure to update the Makefile.windows accordingly (see next section).

## Configuring Makefile.windows

Open `Makefile.windows` in a text editor and locate these lines near the top:

```makefile
# SDL2 paths - Users should customize these to match their SDL2 installation
# Default assumes SDL2 is installed in C:/SDL2
SDL2_DIR = C:/SDL2
SDL2_IMAGE_DIR = C:/SDL2
```

**If you installed SDL2 in a different location**, update these paths. For example:
```makefile
SDL2_DIR = C:/dev/SDL2
SDL2_IMAGE_DIR = C:/dev/SDL2
```

Or if you have separate directories:
```makefile
SDL2_DIR = C:/libraries/SDL2
SDL2_IMAGE_DIR = C:/libraries/SDL2_image
```

**Note:** Use forward slashes (/) in the paths, even on Windows, as this is compatible with MinGW's make.

## Building the Game

Once MinGW and SDL2 are properly installed and configured:

1. Open Command Prompt or PowerShell
2. Navigate to the game directory:
   ```cmd
   cd path\to\CPP_version_of_JavaGame
   ```
3. Build the game using mingw32-make:
   ```cmd
   mingw32-make -f Makefile.windows
   ```

The compilation process will:
- Create an `obj` directory for object files
- Compile all source files
- Link them into `Game.exe`

**Expected Output:**
```
g++ -std=c++11 -Wall -Wextra -g -IC:/SDL2/include -I$(SRC_DIR) -c src/main.cpp -o obj/main.o
...
g++ obj/main.o obj/Game.o ... -o Game.exe -LC:/SDL2/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image
```

## Running the Game

### Running from the Project Directory

If you're running the game from the project directory where it was built:

1. Copy the required DLL files from `C:\SDL2\bin\` to the game directory:
   ```cmd
   copy C:\SDL2\bin\SDL2.dll .
   copy C:\SDL2\bin\SDL2_image.dll .
   copy C:\SDL2\bin\libpng16-16.dll .
   copy C:\SDL2\bin\zlib1.dll .
   ```

2. Run the game:
   ```cmd
   Game.exe
   ```

### Distributing the Game

When distributing your game to other users, you need to include the necessary DLL files:

1. Create a distribution folder
2. Copy `Game.exe` and all required assets (sprites, maps, etc.)
3. Include these DLL files from `C:\SDL2\bin\`:
   - SDL2.dll
   - SDL2_image.dll
   - libpng16-16.dll (or similar, needed by SDL2_image)
   - zlib1.dll (needed by libpng)
   - Any other DLLs that SDL2_image depends on

Your distribution folder structure should look like:
```
YourGame\
├── Game.exe
├── SDL2.dll
├── SDL2_image.dll
├── libpng16-16.dll
├── zlib1.dll
├── maps\
│   └── map01.txt
└── sprites\
    └── ... (sprite files)
```

## Troubleshooting

### Error: "sdl2-config: command not found"

This error means you're trying to use the Linux Makefile on Windows. Make sure you're using:
```cmd
mingw32-make -f Makefile.windows
```

### Error: "No such file or directory" for SDL headers

**Cause:** The SDL2 paths in Makefile.windows are incorrect.

**Solution:**
1. Verify that `C:\SDL2\include\SDL2\SDL.h` exists (or wherever you installed SDL2)
2. Update the `SDL2_DIR` and `SDL2_IMAGE_DIR` paths in Makefile.windows
3. Make sure you're using forward slashes in the paths

### Error: "undefined reference to 'SDL_Init'" or similar linker errors

**Cause:** The linker cannot find the SDL2 library files.

**Solution:**
1. Verify that `C:\SDL2\lib\libSDL2.a` and `libSDL2main.a` exist
2. Check that the `SDL2_DIR` path in Makefile.windows points to the correct location
3. Ensure you're using the MinGW version of SDL2 (not MSVC version)

### Error: "The program can't start because SDL2.dll is missing"

**Cause:** The SDL2 DLL files are not in the same directory as Game.exe or in your system PATH.

**Solution:**
1. Copy all required DLL files from `C:\SDL2\bin\` to the same directory as `Game.exe`
2. Or add `C:\SDL2\bin\` to your system PATH environment variable

### Error: "mingw32-make: command not found"

**Cause:** MinGW is not installed or not in your PATH.

**Solution:**
1. Verify MinGW installation: `g++ --version` in Command Prompt
2. If not found, reinstall MinGW and add it to your PATH
3. For MSYS2 users, the command might be just `make` instead of `mingw32-make`

### Compilation errors with "cannot find -lSDL2"

**Cause:** The library files are not in the expected location or have different names.

**Solution:**
1. Check if the library files in your `SDL2\lib` folder are named correctly:
   - Should be `libSDL2.a` (not `SDL2.lib`)
   - Should be `libSDL2main.a`
   - Should be `libSDL2_image.a`
2. Make sure you downloaded the MinGW version, not the Visual Studio version

### Build succeeds but game crashes on startup

**Cause:** Architecture mismatch (32-bit vs 64-bit).

**Solution:**
1. Ensure your MinGW compiler, SDL2 libraries, and Windows are all the same architecture
2. For 64-bit: Use x86_64-w64-mingw32 folder from SDL2 download
3. For 32-bit: Use i686-w64-mingw32 folder from SDL2 download

## Cleaning Build Artifacts

To clean the build and remove all object files and executables:

```cmd
mingw32-make -f Makefile.windows clean
```

## Rebuilding from Scratch

To clean and rebuild everything:

```cmd
mingw32-make -f Makefile.windows rebuild
```

## Additional Resources

- [SDL2 Documentation](https://wiki.libsdl.org/)
- [MinGW-w64 Documentation](https://www.mingw-w64.org/)
- [MSYS2 Documentation](https://www.msys2.org/)

## Getting Help

If you encounter issues not covered in this guide:
1. Check that all paths in Makefile.windows are correct
2. Verify that you have the MinGW (not MSVC) versions of SDL2 libraries
3. Ensure all DLL files are in the correct location
4. Check the issue tracker on GitHub for similar problems
