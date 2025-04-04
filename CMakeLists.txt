cmake_minimum_required(VERSION 3.10)
project(FightGPT)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set SFML paths
set(SFML_DIR "/opt/homebrew/opt/sfml@2/lib/cmake/SFML")

# Find SFML
find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

# Add source files
add_executable(FightGPT
    src/main.cpp
    src/NameInputState.cpp
    src/CharacterSelectionState.cpp
    src/StoryState.cpp
    src/GamePlayState.cpp
    src/GameLogic.cpp
)

# Set include directories for the target
target_include_directories(FightGPT
    PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    /opt/homebrew/opt/sfml@2/include
)

# Link SFML libraries
target_link_libraries(FightGPT sfml-graphics sfml-window sfml-system)

# Copy assets to build directory
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

# Enable warnings and treat them as errors
if(MSVC)
    target_compile_options(FightGPT PRIVATE /W4)
else()
    target_compile_options(FightGPT PRIVATE -Wall -Wextra)
endif() 