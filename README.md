# FightGPT

A roguelike RPG game where you must defeat the evil boss to escape from a corrupted realm.

## Prerequisites

Before building the game, make sure you have the following installed:

- CMake (version 3.10 or higher)
- C++ compiler with C++17 support
- SFML 2.6 library

### Installing SFML

#### macOS (using Homebrew)
```bash
brew install sfml@2
```

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install libsfml-dev
```

#### Windows
1. Download SFML 2.6 from https://www.sfml-dev.org/download.php
2. Extract it to a known location
3. Add the SFML bin directory to your PATH environment variable

## Building the Game

1. Clone the repository:
```bash
git clone [repository-url]
cd FightGPT
```

2. Create a build directory:
```bash
mkdir build
cd build
```

3. Configure with CMake:

#### macOS/Linux
```bash
cmake ..
```

#### Windows
```bash
cmake .. -DSFML_DIR=path/to/SFML/lib/cmake/SFML
```

4. Build the game:

#### macOS/Linux
```bash
make
```

#### Windows (using Visual Studio)
```bash
cmake --build . --config Release
```

## Running the Game

From the build directory:

#### macOS/Linux
```bash
./FightGPT
```

#### Windows
```bash
.\Release\FightGPT.exe
```

## Game Controls

- Arrow keys: Move character/Navigate menus
- Enter: Confirm selection
- A: Attack in combat
- E: Try to escape from combat
- I: Access inventory during combat
- 1-4: Use items from inventory
- ESC: Exit game

## Character Classes

- **Knight**: High HP and Defense, balanced Attack
- **Mage**: High Attack Power, low Defense
- **Archer**: High Speed and Avoidance, medium Attack

## Building from Source Code

If you want to modify the source code, the project structure is as follows:

```
FightGPT/
├── include/         # Header files
├── src/            # Source files
├── assets/         # Game assets (fonts, images)
├── build/          # Build directory (created during build)
└── CMakeLists.txt  # CMake configuration
```

## Troubleshooting

### Common Issues

1. **SFML not found**
   - Make sure SFML is installed
   - Check if SFML_DIR is set correctly in CMake
   - On macOS, verify Homebrew installation of SFML

2. **Build errors**
   - Ensure you have a C++17 compatible compiler
   - Check if all dependencies are installed
   - Try cleaning the build directory and rebuilding

3. **Runtime errors**
   - Verify that assets are copied to the build directory
   - Check if SFML dynamic libraries are in the system path
