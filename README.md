# CPP Version of JavaGame

A C++ implementation of a 2D game using SDL2.

## Features

- 2D game engine built with SDL2
- Entity Component System (ECS) architecture
- Sprite animation system
- Collision detection
- Map/level support
- Camera system

## Building the Game

### Linux/Unix

Requirements:
- g++ compiler with C++11 support
- SDL2 development libraries
- SDL2_image development libraries

Install dependencies on Ubuntu/Debian:
```bash
sudo apt-get install libsdl2-dev libsdl2-image-dev
```

Build the game:
```bash
make
```

Run the game:
```bash
./Game
```

### Windows

**The project now supports building on Windows using MinGW!**

For detailed Windows build instructions, please see **[WINDOWS_SETUP.md](WINDOWS_SETUP.md)**.

**Quick Start for Windows:**
1. Install MinGW and SDL2 (see WINDOWS_SETUP.md for details)
2. Build the game:
   ```cmd
   mingw32-make -f Makefile.windows
   ```
3. Run the game:
   ```cmd
   Game.exe
   ```

## Project Structure

- `src/` - Source code files
  - `ECS/` - Entity Component System components
  - `Environment/` - Environment-related code
- `sprites/` - Game sprite images
- `maps/` - Level/map data files
- `Makefile` - Build configuration for Linux/Unix
- `Makefile.windows` - Build configuration for Windows/MinGW

## Clean Build

Linux/Unix:
```bash
make clean
```

Windows:
```cmd
mingw32-make -f Makefile.windows clean
```

## License

See repository for license information.
